/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mpvplayer.h"
#include "mpvbackend.h"
#include "mpvqthelper.h"
#include "mpvvideotexturenode.h"
#include <backendinterface.h>
#include "include/mpv/render.h"
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtQuick/qquickwindow.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

static inline void wakeup(void *ctx)
{
    Q_ASSERT(ctx);
    if (!ctx) {
        return;
    }
    // This callback is invoked from any mpv thread (but possibly also
    // recursively from a thread that is calling the mpv API). Just notify
    // the Qt GUI thread to wake up (so that it can process events with
    // mpv_wait_event()), and return as quickly as possible.
    QMetaObject::invokeMethod(static_cast<MPVPlayer *>(ctx), "hasMpvEvents", Qt::QueuedConnection);
}

MPVPlayer::MPVPlayer(QQuickItem *parent) : MediaPlayer(parent)
{
    initialize();
}

MPVPlayer::~MPVPlayer()
{
    deinitialize();
}

void MPVPlayer::initialize()
{
    qCDebug(lcQMPMPV) << "Initializing the MPV backend ...";

    if (!MPV::Qt::isLibmpvAvailable()) {
        qFatal("libmpv is not available.");
    }

    m_mpv = mpv_create();
    Q_ASSERT(m_mpv);
    if (!m_mpv) {
        qFatal("Failed to create the mpv instance.");
    }

    if (!m_livePreview) {
        qCDebug(lcQMPMPV) << "Player created.";
    }

    if (!mpvSetProperty(QStringLiteral("input-default-bindings"), false)) {
        qCWarning(lcQMPMPV) << "Failed to set \"input-default-bindings\" to \"false\".";
    }
    if (!mpvSetProperty(QStringLiteral("input-vo-keyboard"), false)) {
        qCWarning(lcQMPMPV) << "Failed to set \"input-vo-keyboard\" to \"false\".";
    }
    if (!mpvSetProperty(QStringLiteral("input-cursor"), false)) {
        qCWarning(lcQMPMPV) << "Failed to set \"input-cursor\" to \"false\".";
    }
    if (!mpvSetProperty(QStringLiteral("cursor-autohide"), false)) {
        qCWarning(lcQMPMPV) << "Failed to set \"cursor-autohide\" to \"false\".";
    }
    if (!mpvSetProperty(QStringLiteral("vo"), QStringLiteral("libmpv"))) {
        qCWarning(lcQMPMPV) << "Failed to set \"vo\" to \"libmpv\".";
    }
    if (!mpvSetProperty(QStringLiteral("ytdl"), true)) {
        qCWarning(lcQMPMPV) << "Failed to set \"ytdl\" to \"true\".";
    }
    static const QString clientName = QStringLiteral("QtMediaPlayer");
    if (!mpvSetProperty(QStringLiteral("audio-client-name"), clientName)) {
        qCWarning(lcQMPMPV) << "Failed to set \"audio-client-name\" to" << clientName;
    }
    if (!mpvSetProperty(QStringLiteral("load-scripts"), true)) {
        qCWarning(lcQMPMPV) << "Failed to set \"load-scripts\" to \"true\".";
    }
    if (!mpvSetProperty(QStringLiteral("screenshot-format"), QStringLiteral("png"))) {
        qCWarning(lcQMPMPV) << "Failed to set \"screenshot-format\" to \"png\".";
    }
    const QString curDirPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
    if (!mpvSetProperty(QStringLiteral("screenshot-directory"), curDirPath)) {
        qCWarning(lcQMPMPV) << "Failed to set \"screenshot-directory\" to" << curDirPath;
    }
    // Default to software decoding.
    if (!mpvSetProperty(QStringLiteral("hwdec"), QStringLiteral("no"))) {
        qCWarning(lcQMPMPV) << "Failed to set \"hwdec\" to \"no\".";
    }

    auto it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString propName = it.key();
        if (!mpvObserveProperty(propName)) {
            qCWarning(lcQMPMPV) << "Failed to observe property" << propName;
        }
        ++it;
    }

    // From this point on, the wakeup function will be called. The callback
    // can come from any thread, so we use the QueuedConnection mechanism to
    // relay the wakeup in a thread-safe way.
    connect(this, &MPVPlayer::hasMpvEvents, this, &MPVPlayer::handleMpvEvents, Qt::QueuedConnection);

    mpv_set_wakeup_callback(m_mpv, wakeup, this);

    if (mpv_initialize(m_mpv) < 0) {
        qFatal("Failed to initialize the mpv player.");
    }

    connect(this, &MPVPlayer::onUpdate, this, &MPVPlayer::doUpdate, Qt::QueuedConnection);

    connect(this, &MPVPlayer::playbackStateChanged, this, [this](){
        if (isPlaying()) {
            Q_EMIT playing();
        }
        if (isPaused()) {
            Q_EMIT paused();
        }
        if (isStopped()) {
            m_loaded = false;
            m_mediaStatus = {};
            const QUrl url = m_source;
            const qint64 pos = m_lastPosition;
            Q_EMIT stopped();
            Q_EMIT stoppedWithPosition(url, pos);
            m_source.clear();
            Q_EMIT sourceChanged();
            m_lastPosition = 0;
        }
    });
    connect(this, &MPVPlayer::positionChanged, this, [this](){
        m_lastPosition = position();
    });

    connect(this, &MPVPlayer::rendererReadyChanged, this, [this](){
        if (!m_rendererReady) {
            return;
        }
        if (!m_cachedUrl.isValid()) {
            return;
        }
        setSource(m_cachedUrl);
        m_cachedUrl.clear();
    });
}

void MPVPlayer::deinitialize()
{
    if (!isStopped()) {
        stop();
    }
    // Only initialized if something got drawn
    if (m_mpv_gl) {
        mpv_render_context_free(m_mpv_gl);
        m_mpv_gl = nullptr;
    }
    if (m_mpv) {
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
    if (!m_livePreview) {
        qCDebug(lcQMPMPV) << "Player destroyed.";
    }
}

void MPVPlayer::on_update(void *ctx)
{
    Q_ASSERT(ctx);
    if (!ctx) {
        return;
    }
    Q_EMIT static_cast<MPVPlayer *>(ctx)->onUpdate();
}

QString MPVPlayer::backendName() const
{
    return metaData_mpv().value(kName).toString();
}

QString MPVPlayer::backendVersion() const
{
    return metaData_mpv().value(kVersion).toString();
}

QString MPVPlayer::backendAuthors() const
{
    return metaData_mpv().value(kAuthors).toString();
}

QString MPVPlayer::backendCopyright() const
{
    return metaData_mpv().value(kCopyright).toString();
}

QString MPVPlayer::backendLicenses() const
{
    return metaData_mpv().value(kLicenses).toString();
}

QString MPVPlayer::backendHomepage() const
{
    return metaData_mpv().value(kHomepage).toString();
}

QString MPVPlayer::ffmpegVersion() const
{
    return metaData_mpv().value(kFFmpegVersion).toString();
}

QString MPVPlayer::ffmpegConfiguration() const
{
    return metaData_mpv().value(kFFmpegConfiguration).toString();
}

// Connected to onUpdate() signal makes sure it runs on the GUI thread
void MPVPlayer::doUpdate()
{
    update();
}

void MPVPlayer::processMpvLogMessage(void *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (m_livePreview) {
        return;
    }
    const auto e = static_cast<mpv_event_log_message *>(event);
    // The log message from libmpv contains new line. Remove it.
    const QString message = QString::fromUtf8(e->text).trimmed();
    if (message.isEmpty()) {
        return;
    }
    switch (e->log_level) {
    case MPV_LOG_LEVEL_V:
    case MPV_LOG_LEVEL_DEBUG:
    case MPV_LOG_LEVEL_TRACE:
        qCDebug(lcQMPMPV) << message;
        break;
    case MPV_LOG_LEVEL_WARN:
        qCWarning(lcQMPMPV) << message;
        break;
    case MPV_LOG_LEVEL_ERROR:
    case MPV_LOG_LEVEL_FATAL:
        qCCritical(lcQMPMPV) << message;
        break;
    case MPV_LOG_LEVEL_INFO:
        qCInfo(lcQMPMPV) << message;
        break;
    default:
        qCDebug(lcQMPMPV) << message;
        break;
    }
}

void MPVPlayer::processMpvPropertyChange(void *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    const auto e = static_cast<mpv_event_property *>(event);
    const QString name = QString::fromUtf8(e->name);
    if (!propertyBlackList.contains(name) && !m_livePreview) {
        qCDebug(lcQMPMPV) << name << "-->" << mpvGetProperty(name, true);
    }
    if (properties.contains(name)) {
        const QList<const char *> signalNames = properties.value(name);
        if (!signalNames.isEmpty()) {
            for (auto &&signalName : qAsConst(signalNames)) {
                if (signalName) {
                    QMetaObject::invokeMethod(this, signalName);
                }
            }
        }
    }
}

bool MPVPlayer::isLoaded() const
{
    return m_loaded;
}

bool MPVPlayer::isPlaying() const
{
    return playbackState() == PlaybackState::Playing;
}

bool MPVPlayer::isPaused() const
{
    return playbackState() == PlaybackState::Paused;
}

bool MPVPlayer::isStopped() const
{
    return playbackState() == PlaybackState::Stopped;
}

bool MPVPlayer::rotateImage(const qreal value)
{
    if (isStopped()) {
        return false;
    }
    if ((value <= 0.0) || (value >= 360.0)) {
        qCWarning(lcQMPMPV) << "The rotation degree should be in the (0, 360) range.";
        return false;
    }
    if (!mpvSetProperty(QStringLiteral("video-rotate"), value)) {
        qCWarning(lcQMPMPV) << "Failed to set \"video-rotate\" to" << value;
        return false;
    }
    return true;
}

bool MPVPlayer::scaleImage(const qreal value)
{
    if (isStopped()) {
        return false;
    }
    if (value <= 0.0) {
        qCWarning(lcQMPMPV) << "The scale factor should be greater than zero.";
        return false;
    }
    if (!mpvSetProperty(QStringLiteral("video-scale-x"), value)) {
        qCWarning(lcQMPMPV) << "Failed to set \"video-scale-x\" to" << value;
        return false;
    }
    if (!mpvSetProperty(QStringLiteral("video-scale-y"), value)) {
        qCWarning(lcQMPMPV) << "Failed to set \"video-scale-y\" to" << value;
        return false;
    }
    return true;
}

void MPVPlayer::videoReconfig()
{
    Q_EMIT videoSizeChanged();
    // What else to do ?
}

void MPVPlayer::audioReconfig()
{
    // TODO
}

bool MPVPlayer::mpvSendCommand(const QVariant &arguments)
{
    Q_ASSERT(m_mpv);
    if (!m_mpv) {
        return false;
    }
    if (!arguments.isValid()) {
        return false;
    }
    if (!m_livePreview) {
        qCDebug(lcQMPMPV) << "Command:" << arguments;
    }
    const int errorCode = MPV::Qt::get_error(MPV::Qt::command(m_mpv, arguments));
    if ((errorCode < 0) && !m_livePreview) {
        qCWarning(lcQMPMPV) << "Failed to send command" << arguments << ':' << mpv_error_string(errorCode);
    }
    return (errorCode >= 0);
}

bool MPVPlayer::mpvSetProperty(const QString &name, const QVariant &value)
{
    Q_ASSERT(m_mpv);
    if (!m_mpv) {
        return false;
    }
    if (name.isEmpty() || !value.isValid()) {
        return false;
    }
    if (!m_livePreview) {
        qCDebug(lcQMPMPV) << name << "-->" << value;
    }
    const int errorCode = MPV::Qt::set_property(m_mpv, name, value);
    if ((errorCode < 0) && !m_livePreview) {
        qCWarning(lcQMPMPV) << "Failed to change property" << name
                            << "to" << value << ':' << mpv_error_string(errorCode);
    }
    return (errorCode >= 0);
}

QVariant MPVPlayer::mpvGetProperty(const QString &name, const bool silent, bool *ok) const
{
    Q_ASSERT(m_mpv);
    if (!m_mpv) {
        return false;
    }
    if (ok) {
        *ok = false;
    }
    if (name.isEmpty()) {
        return {};
    }
    const QVariant result = MPV::Qt::get_property(m_mpv, name);
    const int errorCode = MPV::Qt::get_error(result);
    if (!result.isValid() || (errorCode < 0)) {
        if (!silent && !m_livePreview) {
            qCWarning(lcQMPMPV) << "Failed to query property" << name << ':' << mpv_error_string(errorCode);
        }
    } else {
        if (ok) {
            *ok = true;
        }
        /*if ((name != "time-pos") && (name != "duration")) {
            qCDebug(lcQMPMPV) << "Querying a property from mpv:" << name << "result:" << result;
        }*/
    }
    return result;
}

bool MPVPlayer::mpvObserveProperty(const QString &name)
{
    Q_ASSERT(m_mpv);
    if (!m_mpv) {
        return false;
    }
    if (name.isEmpty()) {
        return false;
    }
    const int errorCode = mpv_observe_property(m_mpv, 0, qUtf8Printable(name), MPV_FORMAT_NONE);
    if ((errorCode < 0) && !m_livePreview) {
        qCWarning(lcQMPMPV) << "Failed to observe property" << name << ':' << mpv_error_string(errorCode);
    }
    return (errorCode >= 0);
}

QUrl MPVPlayer::source() const
{
    return m_source;
}

QString MPVPlayer::fileName() const
{
    return isStopped() ? QString{} : mpvGetProperty(QStringLiteral("filename")).toString();
}

QSizeF MPVPlayer::videoSize() const
{
    if (isStopped()) {
        return {};
    }
    return {mpvGetProperty(QStringLiteral("video-out-params/dw")).toReal(),
            mpvGetProperty(QStringLiteral("video-out-params/dh")).toReal()};
}

PlaybackState MPVPlayer::playbackState() const
{
    const bool stopped = mpvGetProperty(QStringLiteral("idle-active")).toBool();
    const bool paused = mpvGetProperty(QStringLiteral("pause")).toBool();
    return stopped ? PlaybackState::Stopped : (paused ? PlaybackState::Paused : PlaybackState::Playing);
}

MediaStatus MPVPlayer::mediaStatus() const
{
    return m_mediaStatus;
}

LogLevel MPVPlayer::logLevel() const
{
    const QString level = mpvGetProperty(QStringLiteral("msg-level")).toString();
    if (level.isEmpty() || (level == QStringLiteral("no")) || (level == QStringLiteral("off"))) {
        return LogLevel::Off;
    }
    const QString actualLevel = level.right(level.length() - level.lastIndexOf(u'=') - 1);
    if (actualLevel.isEmpty() || (actualLevel == QStringLiteral("no"))
        || (actualLevel == QStringLiteral("off"))) {
        return LogLevel::Off;
    }
    if ((actualLevel == QStringLiteral("v")) || (actualLevel == QStringLiteral("debug"))
        || (actualLevel == QStringLiteral("trace"))) {
        return LogLevel::Debug;
    }
    if (actualLevel == QStringLiteral("warn")) {
        return LogLevel::Warning;
    }
    if (actualLevel == QStringLiteral("error")) {
        return LogLevel::Critical;
    }
    if (actualLevel == QStringLiteral("fatal")) {
        return LogLevel::Fatal;
    }
    if (actualLevel == QStringLiteral("info")) {
        return LogLevel::Info;
    }
    return LogLevel::Debug;
}

qint64 MPVPlayer::duration() const
{
    return isStopped() ? 0 : (mpvGetProperty(QStringLiteral("duration")).toLongLong() * 1000);
}

qint64 MPVPlayer::position() const
{
    return isStopped() ? 0 : (mpvGetProperty(QStringLiteral("time-pos")).toLongLong() * 1000);
}

qreal MPVPlayer::volume() const
{
    return (mpvGetProperty(QStringLiteral("volume")).toReal() / 100.0);
}

bool MPVPlayer::mute() const
{
    return mpvGetProperty(QStringLiteral("mute")).toBool();
}

bool MPVPlayer::seekable() const
{
    return isStopped() ? false : mpvGetProperty(QStringLiteral("seekable")).toBool();
}

bool MPVPlayer::hardwareDecoding() const
{
    // Querying "hwdec" itself will return empty string.
    const QString hwdec = mpvGetProperty(QStringLiteral("hwdec-current")).toString();
    return (!hwdec.isEmpty() && (hwdec != QStringLiteral("no")) && (hwdec != QStringLiteral("off")));
}

qreal MPVPlayer::aspectRatio() const
{
    const qreal result = mpvGetProperty(QStringLiteral("video-out-params/aspect")).toReal();
    return ((result > 0.0) ? result : (16.0 / 9.0));
}

qreal MPVPlayer::playbackRate() const
{
    return mpvGetProperty(QStringLiteral("speed")).toReal();
}

QString MPVPlayer::snapshotFormat() const
{
    return mpvGetProperty(QStringLiteral("screenshot-format")).toString();
}

QString MPVPlayer::snapshotTemplate() const
{
    return mpvGetProperty(QStringLiteral("screenshot-template")).toString();
}

QUrl MPVPlayer::snapshotDirectory() const
{
    return mpvGetProperty(QStringLiteral("screenshot-directory")).toString();
}

QString MPVPlayer::filePath() const
{
    return isStopped() ? QString{} : QDir::toNativeSeparators(mpvGetProperty(QStringLiteral("path")).toString());
}

MediaTracks MPVPlayer::mediaTracks() const
{
    if (isStopped()) {
        return {};
    }
    const QVariantList trackList = mpvGetProperty(QStringLiteral("track-list")).toList();
    if (trackList.isEmpty()) {
        return {};
    }
    MediaTracks result = {};
    for (auto &&track : qAsConst(trackList)) {
        const QVariantMap trackInfo = track.toMap();
        if (trackInfo.isEmpty()) {
            continue;
        }
        const QString type = trackInfo.value(QStringLiteral("type")).toString();
        if ((type != QStringLiteral("video"))
            && (type != QStringLiteral("audio")) && (type != QStringLiteral("sub"))) {
            continue;
        }
        QVariantHash info = {};
        info.insert(QStringLiteral("id"), trackInfo.value(QStringLiteral("id")));
        info.insert(QStringLiteral("type"), type);
        info.insert(QStringLiteral("src-id"), trackInfo.value(QStringLiteral("src-id")));
        const QString lang = trackInfo.value(QStringLiteral("lang")).toString();
        info.insert(QStringLiteral("lang"), lang);
        const QString title = trackInfo.value(QStringLiteral("title")).toString();
        if (title.isEmpty()) {
            if (lang != QStringLiteral("und")) {
                info.insert(QStringLiteral("title"), lang);
            } else if (!trackInfo.value(QStringLiteral("external")).toBool()) {
                info.insert(QStringLiteral("title"), QStringLiteral("[internal]"));
            } else {
                info.insert(QStringLiteral("title"), QStringLiteral("[untitled]"));
            }
        } else {
            info.insert(QStringLiteral("title"), title);
        }
        info.insert(QStringLiteral("default"), trackInfo.value(QStringLiteral("default")));
        info.insert(QStringLiteral("forced"), trackInfo.value(QStringLiteral("forced")));
        info.insert(QStringLiteral("codec"), trackInfo.value(QStringLiteral("codec")));
        info.insert(QStringLiteral("external"), trackInfo.value(QStringLiteral("external")));
        info.insert(QStringLiteral("external-filename"), trackInfo.value(QStringLiteral("external-filename")));
        info.insert(QStringLiteral("selected"), trackInfo.value(QStringLiteral("selected")));
        info.insert(QStringLiteral("decoder-desc"), trackInfo.value(QStringLiteral("decoder-desc")));
        info.insert(QStringLiteral("albumart"), trackInfo.value(QStringLiteral("albumart")));
        info.insert(QStringLiteral("image"), trackInfo.value(QStringLiteral("image")));
        info.insert(QStringLiteral("main-selection"), trackInfo.value(QStringLiteral("main-selection")));
        info.insert(QStringLiteral("ff-index"), trackInfo.value(QStringLiteral("ff-index")));
        if (type == QStringLiteral("video")) {
            info.insert(QStringLiteral("demux-w"), trackInfo.value(QStringLiteral("demux-w")));
            info.insert(QStringLiteral("demux-h"), trackInfo.value(QStringLiteral("demux-h")));
            info.insert(QStringLiteral("demux-fps"), trackInfo.value(QStringLiteral("demux-fps")));
            info.insert(QStringLiteral("demux-rotation"), trackInfo.value(QStringLiteral("demux-rotation")));
            info.insert(QStringLiteral("demux-par"), trackInfo.value(QStringLiteral("demux-par")));
            result.video.append(info);
        } else if (type == QStringLiteral("audio")) {
            info.insert(QStringLiteral("demux-channel-count"), trackInfo.value(QStringLiteral("demux-channel-count")));
            info.insert(QStringLiteral("demux-channels"), trackInfo.value(QStringLiteral("demux-channels")));
            info.insert(QStringLiteral("demux-samplerate"), trackInfo.value(QStringLiteral("demux-samplerate")));
            info.insert(QStringLiteral("demux-bitrate"), trackInfo.value(QStringLiteral("demux-bitrate")));
            result.audio.append(info);
        } else if (type == QStringLiteral("sub")) {
            result.subtitle.append(info);
        }
    }
    return result;
}

int MPVPlayer::activeVideoTrack() const
{
    return isStopped() ? 0 : mpvGetProperty(QStringLiteral("vid")).toInt();
}

void MPVPlayer::setActiveVideoTrack(const int value)
{
    if (isStopped()) {
        qCWarning(lcQMPMPV) << "Setting active track before the media is loaded has no effect."
                            << "Please try again later when the media has been loaded successfully.";
        return;
    }
    int track = value;
    if (track < 0) {
        track = 0;
        qCWarning(lcQMPMPV) << "The minimum track number is zero, "
                               "setting active track to a negative number equals to reset to default track.";
    } else {
        const int totalTrackCount = mediaTracks().video.count();
        if (track >= totalTrackCount) {
            track = totalTrackCount - 1;
            qCWarning(lcQMPMPV) << "Total video track count is" << totalTrackCount
                                << ". Can't set active track to a number greater or equal to it.";
        }
    }
    if (activeVideoTrack() == track) {
        return;
    }
    if (!mpvSetProperty(QStringLiteral("vid"), track)) {
        qCWarning(lcQMPMPV) << "Failed to set \"vid\" to" << track;
    }
}

int MPVPlayer::activeAudioTrack() const
{
    return isStopped() ? 0 : mpvGetProperty(QStringLiteral("aid")).toInt();
}

void MPVPlayer::setActiveAudioTrack(const int value)
{
    if (isStopped()) {
        qCWarning(lcQMPMPV) << "Setting active track before the media is loaded has no effect."
                            << "Please try again later when the media has been loaded successfully.";
        return;
    }
    int track = value;
    if (track < 0) {
        track = 0;
        qCWarning(lcQMPMPV) << "The minimum track number is zero, "
                               "setting active track to a negative number equals to reset to default track.";
    } else {
        const int totalTrackCount = mediaTracks().audio.count();
        if (track >= totalTrackCount) {
            track = totalTrackCount - 1;
            qCWarning(lcQMPMPV) << "Total audio track count is" << totalTrackCount
                                << ". Can't set active track to a number greater or equal to it.";
        }
    }
    if (activeAudioTrack() == track) {
        return;
    }
    if (!mpvSetProperty(QStringLiteral("aid"), track)) {
        qCWarning(lcQMPMPV) << "Failed to set \"aid\" to" << track;
    }
}

int MPVPlayer::activeSubtitleTrack() const
{
    return isStopped() ? 0 : mpvGetProperty(QStringLiteral("sid")).toInt();
}

void MPVPlayer::setActiveSubtitleTrack(const int value)
{
    if (isStopped()) {
        qCWarning(lcQMPMPV) << "Setting active track before the media is loaded has no effect."
                            << "Please try again later when the media has been loaded successfully.";
        return;
    }
    int track = value;
    if (track < 0) {
        track = 0;
        qCWarning(lcQMPMPV) << "The minimum track number is zero, "
                               "setting active track to a negative number equals to reset to default track.";
    } else {
        const int totalTrackCount = mediaTracks().subtitle.count();
        if (track >= totalTrackCount) {
            track = totalTrackCount - 1;
            qCWarning(lcQMPMPV) << "Total subtitle track count is" << totalTrackCount
                                << ". Can't set active track to a number greater or equal to it.";
        }
    }
    if (activeSubtitleTrack() == track) {
        return;
    }
    if (!mpvSetProperty(QStringLiteral("sid"), track)) {
        qCWarning(lcQMPMPV) << "Failed to set \"sid\" to" << track;
    }
}

bool MPVPlayer::rendererReady() const
{
    return m_rendererReady;
}

Chapters MPVPlayer::chapters() const
{
    if (isStopped()) {
        return {};
    }
    const QVariantList chapterList = mpvGetProperty(QStringLiteral("chapter-list")).toList();
    if (chapterList.isEmpty()) {
        return {};
    }
    Chapters result = {};
    for (auto &&chapter : qAsConst(chapterList)) {
        const QVariantMap chapterInfo = chapter.toMap();
        if (chapterInfo.isEmpty()) {
            continue;
        }
        ChapterInfo info = {};
        info.title = chapterInfo.value(QStringLiteral("title")).toString();
        info.startTime = qRound64(chapterInfo.value(QStringLiteral("time")).toReal() * 1000.0);
        info.endTime = 0; // ### FIXME
        result.append(info);
    }
    return result;
}

MetaData MPVPlayer::metaData() const
{
    if (isStopped()) {
        return {};
    }
    const QVariantMap md = mpvGetProperty(QStringLiteral("metadata")).toMap();
    if (md.isEmpty()) {
        return {};
    }
    MetaData result = {};
    auto it = md.constBegin();
    while (it != md.constEnd()) {
        result.insert(it.key(), it.value());
        ++it;
    }
    return result;
}

bool MPVPlayer::livePreview() const
{
    return m_livePreview;
}

void MPVPlayer::play()
{
    if (!m_source.isValid() || m_livePreview) {
        return;
    }
    if (!mpvSetProperty(QStringLiteral("pause"), false)) {
        qCWarning(lcQMPMPV) << "Failed to set \"pause\" to \"false\".";
    }
}

void MPVPlayer::pause()
{
    if (!m_source.isValid()) {
        return;
    }
    if (!mpvSetProperty(QStringLiteral("pause"), true)) {
        qCWarning(lcQMPMPV) << "Failed to set \"pause\" to \"true\".";
    }
}

void MPVPlayer::stop()
{
    if (!m_source.isValid()) {
        return;
    }
    if (!mpvSendCommand(QVariantList{QStringLiteral("stop")})) {
        qCWarning(lcQMPMPV) << "Failed to send command \"stop\".";
    }
}

void MPVPlayer::seek(const qint64 value)
{
    if (isStopped() || (position() == value)) {
        return;
    }
    if (value < 0) {
        qCWarning(lcQMPMPV) << "Media start time is 0, however, the user is trying to seek to" << value;
        return;
    }
    const qint64 _duration = duration();
    if (value > _duration) {
        qCWarning(lcQMPMPV) << "Media duration is" << _duration
                            << ", however, the user is trying to seek to" << value;
        return;
    }
    if (!mpvSendCommand(QVariantList{QStringLiteral("seek"),
                                     qRound64(static_cast<qreal>(value) / 1000.0),
                                     QStringLiteral("absolute")})) {
        qCWarning(lcQMPMPV) << "Failed to send command \"seek\".";
    }
}

void MPVPlayer::snapshot()
{
    if (isStopped()) {
        return;
    }
    // Replace "subtitles" with "video" if you don't want to include subtitles when screenshotting.
    if (!mpvSendCommand(QVariantList{QStringLiteral("screenshot"), QStringLiteral("subtitles")})) {
        qCWarning(lcQMPMPV) << "Failed to send command \"screenshot\".";
    }
}

void MPVPlayer::setSource(const QUrl &value)
{
    if (!m_rendererReady) {
        m_cachedUrl = value;
        return;
    }
    if (value.isEmpty()) {
        qCDebug(lcQMPMPV) << "Empty source is set, playback stopped.";
        stop();
        return;
    }
    if (!value.isValid()) {
        qCWarning(lcQMPMPV) << "The given URL" << value << "is invalid.";
        return;
    }
    if (QString::compare(value.scheme(), QStringLiteral("qrc"), Qt::CaseInsensitive) == 0) {
        qCWarning(lcQMPMPV) << "Currently embeded resource is not supported.";
        return;
    }
    const QString filename = value.fileName();
    if (filename.isEmpty()) {
        qCWarning(lcQMPMPV) << "The source url" << value << "doesn't contain a filename.";
        return;
    }
    if (!isMediaFile(filename)) {
        qCWarning(lcQMPMPV) << "The source url" << value << "doesn't seem to be a multimedia file.";
        return;
    }
    if (value == m_source) {
        if (isStopped() && !m_livePreview) {
            play();
        }
        return;
    }
    stop();
    const bool result = mpvSendCommand(QVariantList{QStringLiteral("loadfile"), value.isLocalFile()
                                                        ? QDir::toNativeSeparators(value.toLocalFile())
                                                        : value.toString()});
    if (result) {
        if (m_livePreview || !m_autoStart) {
            if (!mpvSetProperty(QStringLiteral("pause"), true)) {
                qCWarning(lcQMPMPV) << "Failed to set \"pause\" to \"true\".";
            }
        }
        m_source = value;
        Q_EMIT sourceChanged();
    }
}

void MPVPlayer::setMute(const bool value)
{
    if (mute() == value) {
        return;
    }
    if (!mpvSetProperty(QStringLiteral("mute"), value)) {
        qCWarning(lcQMPMPV) << "Failed to set \"mute\" to" << value;
    }
}

void MPVPlayer::setPlaybackState(const PlaybackState value)
{
    if (playbackState() == value) {
        return;
    }
    switch (value) {
    case PlaybackState::Stopped:
        stop();
        break;
    case PlaybackState::Paused:
        pause();
        break;
    case PlaybackState::Playing:
        play();
        break;
    }
}

void MPVPlayer::setLogLevel(const LogLevel value)
{
    Q_ASSERT(m_mpv);
    if (!m_mpv) {
        return;
    }
    QString level = QStringLiteral("debug");
    switch (value) {
    case LogLevel::Off:
        level = QStringLiteral("no");
        break;
    case LogLevel::Debug:
        // libmpv's log level: v (verbose) < debug < trace (print all messages)
        // Use "v" to avoid noisy message floods.
        level = QStringLiteral("v");
        break;
    case LogLevel::Warning:
        level = QStringLiteral("warn");
        break;
    case LogLevel::Critical:
        level = QStringLiteral("error");
        break;
    case LogLevel::Fatal:
        level = QStringLiteral("fatal");
        break;
    case LogLevel::Info:
        level = QStringLiteral("info");
        break;
    }
    const bool result1 = mpvSetProperty(QStringLiteral("terminal"), level != QStringLiteral("no"));
    const bool result2 = mpvSetProperty(QStringLiteral("msg-level"), QStringLiteral("all=%1").arg(level));
    const int errorCode = mpv_request_log_messages(m_mpv, qUtf8Printable(level));
    if (result1 && result2 && (errorCode >= 0)) {
        Q_EMIT logLevelChanged();
    } else {
        if (!m_livePreview) {
            qCWarning(lcQMPMPV) << "Failed to change log level to"
                                << level << ':' << mpv_error_string(errorCode);
        }
    }
}

void MPVPlayer::setPosition(const qint64 value)
{
    seek(value);
}

void MPVPlayer::setVolume(const qreal value)
{
    if (qFuzzyCompare(volume(), value)) {
        return;
    }
    if (value < 0.0) {
        qCWarning(lcQMPMPV) << "The minimum volume is 0, however, the user is trying to change it to" << value;
        return;
    }
    if (value > 1.0) {
        qCWarning(lcQMPMPV) << "The maximum volume is 1.0, however, the user is trying to change it to" << value
                   << ". It's allowed but it may cause damaged sound.";
    }
    const int vol = qRound(value * 100.0);
    if (!mpvSetProperty(QStringLiteral("volume"), vol)) {
        qCWarning(lcQMPMPV) << "Failed to set \"volume\" to" << vol;
    }
}

void MPVPlayer::setHardwareDecoding(const bool value)
{
    if (hardwareDecoding() == value) {
        return;
    }
    if (value) {
        static bool warningOnce = false;
        if (!warningOnce) {
            warningOnce = true;
            qCWarning(lcQMPMPV).noquote() << hardwareDecodingWarningText;
        }
    }
    const QString hwdec = (value ? QStringLiteral("auto-safe") : QStringLiteral("no"));
    if (!mpvSetProperty(QStringLiteral("hwdec"), hwdec)) {
        qCWarning(lcQMPMPV) << "Failed to set \"hwdec\" to" << hwdec;
    }
}

bool MPVPlayer::autoStart() const
{
    return m_autoStart;
}

void MPVPlayer::setAutoStart(const bool value)
{
    if (m_autoStart == value) {
        return;
    }
    m_autoStart = value;
    Q_EMIT autoStartChanged();
}

void MPVPlayer::setAspectRatio(const qreal value)
{
    if (qFuzzyCompare(aspectRatio(), value)) {
        return;
    }
    if (value <= 0.0) {
        qCWarning(lcQMPMPV) << "The user is trying to change the aspect ratio to"
                            << value << ", which is not allowed.";
        return;
    }
    if (!mpvSetProperty(QStringLiteral("video-aspect-override"), value)) {
        qCWarning(lcQMPMPV) << "Failed to set \"video-aspect-override\" to" << value;
    }
}

void MPVPlayer::setPlaybackRate(const qreal value)
{
    if (qFuzzyCompare(playbackRate(), value)) {
        return;
    }
    if (value <= 0.0) {
        qCWarning(lcQMPMPV) << "The user is trying to change the playback rate to"
                            << value << ", which is not allowed.";
        return;
    }
    if (!mpvSetProperty(QStringLiteral("speed"), value)) {
        qCWarning(lcQMPMPV) << "Failed to set \"speed\" to" << value;
    }
}

void MPVPlayer::setSnapshotFormat(const QString &value)
{
    if (value.isEmpty() || (snapshotFormat() == value)) {
        return;
    }
    if (!mpvSetProperty(QStringLiteral("screenshot-format"), value)) {
        qCWarning(lcQMPMPV) << "Failed to set \"screenshot-format\" to" << value;
    }
}

void MPVPlayer::setSnapshotTemplate(const QString &value)
{
    if (value.isEmpty() || (snapshotTemplate() == value)) {
        return;
    }
    if (!mpvSetProperty(QStringLiteral("screenshot-template"), value)) {
        qCWarning(lcQMPMPV) << "Failed to set \"screenshot-template\" to" << value;
    }
}

void MPVPlayer::setSnapshotDirectory(const QUrl &value)
{
    if (!value.isValid() || (snapshotDirectory() == value)) {
        return;
    }
    const QString dir = QDir::toNativeSeparators(value.toLocalFile());
    if (!mpvSetProperty(QStringLiteral("screenshot-directory"), dir)) {
        qCWarning(lcQMPMPV) << "Failed to set \"screenshot-directory\" to" << dir;
    }
}

void MPVPlayer::setLivePreview(const bool value)
{
    if (m_livePreview == value) {
        return;
    }
    if (value) {
        setLogLevel(LogLevel::Off);
        if (!mpvSetProperty(QStringLiteral("pause"), true)) {
            qCWarning(lcQMPMPV) << "Failed to set \"pause\" to \"true\".";
        }
        if (!mpvSetProperty(QStringLiteral("mute"), true)) {
            qCWarning(lcQMPMPV) << "Failed to set \"mute\" to \"true\".";
        }
        if (!mpvSetProperty(QStringLiteral("hr-seek"), QStringLiteral("yes"))) {
            qCWarning(lcQMPMPV) << "Failed to set \"hr-seek\" to \"yes\".";
        }
    } else {
        if (!mpvSetProperty(QStringLiteral("hr-seek"), QStringLiteral("default"))) {
            qCWarning(lcQMPMPV) << "Failed to set \"hr-seek\" to \"default\".";
        }
        setLogLevel(LogLevel::Warning); // TODO: back to previous
    }
    m_livePreview = value;
    Q_EMIT livePreviewChanged();
}

FillMode MPVPlayer::fillMode() const
{
    bool ok = false;
    const bool keepaspect = mpvGetProperty(QStringLiteral("keepaspect"), false, &ok).toBool();
    if (!ok) {
        return FillMode::PreserveAspectFit;
    }
    if (!keepaspect) {
        return FillMode::Stretch;
    }
    const QString videoUnscaledStr = mpvGetProperty(QStringLiteral("video-unscaled")).toString();
    if (videoUnscaledStr.isEmpty() || (videoUnscaledStr == QStringLiteral("no"))) {
        return FillMode::PreserveAspectFit;
    }
    return FillMode::PreserveAspectCrop;
}

void MPVPlayer::setFillMode(const FillMode value)
{
    if (fillMode() == value) {
        return;
    }
    switch (value) {
    case FillMode::PreserveAspectFit: {
        if (!mpvSetProperty(QStringLiteral("keepaspect"), true)) {
            qCWarning(lcQMPMPV) << "Failed to set \"keepaspect\" to \"true\".";
        }
        if (!mpvSetProperty(QStringLiteral("video-unscaled"), QStringLiteral("no"))) {
            qCWarning(lcQMPMPV) << "Failed to set \"video-unscaled\" to \"no\".";
        }
    } break;
    case FillMode::PreserveAspectCrop: {
        if (!mpvSetProperty(QStringLiteral("keepaspect"), true)) {
            qCWarning(lcQMPMPV) << "Failed to set \"keepaspect\" to \"true\".";
        }
        if (!mpvSetProperty(QStringLiteral("video-unscaled"), QStringLiteral("yes"))) {
            qCWarning(lcQMPMPV) << "Failed to set \"video-unscaled\" to \"yes\".";
        }
    } break;
    case FillMode::Stretch:
        if (!mpvSetProperty(QStringLiteral("keepaspect"), false)) {
            qCWarning(lcQMPMPV) << "Failed to set \"keepaspect\" to \"false\".";
        }
        break;
    }
}

void MPVPlayer::handleMpvEvents()
{
    Q_ASSERT(m_mpv);
    // Process all events, until the event queue is empty.
    while (m_mpv) {
        const auto event = mpv_wait_event(m_mpv, 0.005);
        if (!event) {
            continue;
        }
        // Nothing happened. Happens on timeouts or sporadic wakeups.
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        bool shouldOutput = true;
        switch (event->event_id) {
        // Happens when the player quits. The player enters a state where it
        // tries to disconnect all clients. Most requests to the player will
        // fail, and the client should react to this and quit with
        // mpv_destroy() as soon as possible.
        case MPV_EVENT_SHUTDOWN:
            break;
        // See mpv_request_log_messages().
        case MPV_EVENT_LOG_MESSAGE:
            processMpvLogMessage(event->data);
            shouldOutput = false;
            break;
        // Reply to a mpv_get_property_async() request.
        // See also mpv_event and mpv_event_property.
        case MPV_EVENT_GET_PROPERTY_REPLY:
            shouldOutput = false;
            break;
        // Reply to a mpv_set_property_async() request.
        // (Unlike MPV_EVENT_GET_PROPERTY, mpv_event_property is not used.)
        case MPV_EVENT_SET_PROPERTY_REPLY:
            shouldOutput = false;
            break;
        // Reply to a mpv_command_async() or mpv_command_node_async() request.
        // See also mpv_event and mpv_event_command.
        case MPV_EVENT_COMMAND_REPLY:
            shouldOutput = false;
            break;
        // Notification before playback start of a file (before the file is
        // loaded).
        case MPV_EVENT_START_FILE:
            m_mediaStatus = MediaStatusFlag::Loading;
            Q_EMIT mediaStatusChanged();
            break;
        // Notification after playback end (after the file was unloaded).
        // See also mpv_event and mpv_event_end_file.
        case MPV_EVENT_END_FILE:
            m_loaded = false;
            m_mediaStatus = (MediaStatusFlag::NoMedia | MediaStatusFlag::Unloaded | MediaStatusFlag::End);
            Q_EMIT mediaStatusChanged();
            break;
        // Notification when the file has been loaded (headers were read
        // etc.), and decoding starts.
        case MPV_EVENT_FILE_LOADED:
            m_loaded = true;
            m_mediaStatus = (MediaStatusFlag::Loaded | MediaStatusFlag::Prepared | MediaStatusFlag::Buffering);
            Q_EMIT mediaStatusChanged();
            Q_EMIT loaded();
            break;
        // Triggered by the script-message input command. The command uses the
        // first argument of the command as client name (see mpv_client_name())
        // to dispatch the message, and passes along all arguments starting from
        // the second argument as strings.
        // See also mpv_event and mpv_event_client_message.
        case MPV_EVENT_CLIENT_MESSAGE:
            break;
        // Happens after video changed in some way. This can happen on
        // resolution changes, pixel format changes, or video filter changes.
        // The event is sent after the video filters and the VO are
        // reconfigured. Applications embedding a mpv window should listen to
        // this event in order to resize the window if needed.
        // Note that this event can happen sporadically, and you should check
        // yourself whether the video parameters really changed before doing
        // something expensive.
        case MPV_EVENT_VIDEO_RECONFIG:
            videoReconfig();
            break;
        // Similar to MPV_EVENT_VIDEO_RECONFIG. This is relatively
        // uninteresting, because there is no such thing as audio output
        // embedding.
        case MPV_EVENT_AUDIO_RECONFIG:
            audioReconfig();
            break;
        // Happens when a seek was initiated. Playback stops. Usually it will
        // resume with MPV_EVENT_PLAYBACK_RESTART as soon as the seek is
        // finished.
        case MPV_EVENT_SEEK:
            m_mediaStatus &= ~MediaStatus(MediaStatusFlag::Buffered);
            m_mediaStatus |= (MediaStatusFlag::Seeking | MediaStatusFlag::Buffering);
            Q_EMIT mediaStatusChanged();
            break;
        // There was a discontinuity of some sort (like a seek), and playback
        // was reinitialized. Usually happens after seeking, or ordered chapter
        // segment switches. The main purpose is allowing the client to detect
        // when a seek request is finished.
        case MPV_EVENT_PLAYBACK_RESTART:
            m_mediaStatus &= ~(MediaStatusFlag::Seeking | MediaStatusFlag::Buffering);
            m_mediaStatus |= MediaStatusFlag::Buffered;
            Q_EMIT mediaStatusChanged();
            break;
        // Event sent due to mpv_observe_property().
        // See also mpv_event and mpv_event_property.
        case MPV_EVENT_PROPERTY_CHANGE:
            processMpvPropertyChange(event->data);
            shouldOutput = false;
            break;
        // Happens if the internal per-mpv_handle ringbuffer overflows, and at
        // least 1 event had to be dropped. This can happen if the client
        // doesn't read the event queue quickly enough with mpv_wait_event(), or
        // if the client makes a very large number of asynchronous calls at
        // once.
        // Event delivery will continue normally once this event was returned
        // (this forces the client to empty the queue completely).
        case MPV_EVENT_QUEUE_OVERFLOW:
            break;
        // Triggered if a hook handler was registered with mpv_hook_add(), and
        // the hook is invoked. If you receive this, you must handle it, and
        // continue the hook with mpv_hook_continue().
        // See also mpv_event and mpv_event_hook.
        case MPV_EVENT_HOOK:
            break;
        default:
            break;
        }
        if (shouldOutput && !m_livePreview) {
            qCDebug(lcQMPMPV) << mpv_event_name(event->event_id) << "event received.";
        }
    }
}

// The beauty of using a true QSGNode: no need for complicated cleanup
// arrangements, unlike in other examples like metalunderqml, because the
// scenegraph will handle destroying the node at the appropriate time.
void MPVPlayer::invalidateSceneGraph() // Called on the render thread when the scenegraph is invalidated
{
    m_node = nullptr;
}

void MPVPlayer::setRendererReady(const bool value)
{
    if (m_rendererReady == value) {
        return;
    }
    m_rendererReady = value;
    Q_EMIT rendererReadyChanged();
}

void MPVPlayer::releaseResources() // Called on the gui thread if the item is removed from scene
{
    m_node = nullptr;
}

QSGNode *MPVPlayer::updatePaintNode(QSGNode *node, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    auto n = static_cast<MPVVideoTextureNode*>(node);
    if (!n && ((width() <= 0) || (height() <= 0))) {
        return nullptr;
    }
    if (!n) {
        m_node = new MPVVideoTextureNode(this);
        n = m_node;
    }
    m_node->sync();
    window()->update(); // Ensure getting to beforeRendering() at some point
    return n;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void MPVPlayer::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
#else
void MPVPlayer::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
#endif
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    MediaPlayer::geometryChange(newGeometry, oldGeometry);
#else
    MediaPlayer::geometryChanged(newGeometry, oldGeometry);
#endif
    if (newGeometry.size() != oldGeometry.size()) {
        update();
    }
}

QTMEDIAPLAYER_END_NAMESPACE
