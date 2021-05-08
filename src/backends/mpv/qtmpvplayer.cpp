/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

#include "qtmpvplayer.h"
#include "qtmpvhelper.h"
#include "qtmpvvideotexturenode.h"
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>

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
    QMetaObject::invokeMethod(static_cast<QTMEDIAPLAYER_PREPEND_NAMESPACE(QtMPVPlayer) *>(ctx), "hasMpvEvents", Qt::QueuedConnection);
}

QTMEDIAPLAYER_BEGIN_NAMESPACE

QtMPVPlayer::QtMPVPlayer(QQuickItem *parent) : QtMediaPlayer(parent)
{
    qDebug() << "Initializing the MPV backend ...";

    // Qt sets the locale in the QGuiApplication constructor, but libmpv
    // requires the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");

    qRegisterMetaType<MPV::Qt::ErrorReturn>();

    m_mpv = MPV::Qt::create();
    if (!m_mpv) {
        qFatal("Failed to create mpv player.");
    }

    if (!m_livePreview) {
        qDebug() << "Player created.";
    }

    mpvSetProperty(QStringLiteral("input-default-bindings"), false);
    mpvSetProperty(QStringLiteral("input-vo-keyboard"), false);
    mpvSetProperty(QStringLiteral("input-cursor"), false);
    mpvSetProperty(QStringLiteral("cursor-autohide"), false);

    auto iterator = properties.cbegin();
    while (iterator != properties.cend()) {
        mpvObserveProperty(iterator.key());
        ++iterator;
    }

    // From this point on, the wakeup function will be called. The callback
    // can come from any thread, so we use the QueuedConnection mechanism to
    // relay the wakeup in a thread-safe way.
    connect(this, &QtMPVPlayer::hasMpvEvents, this, &QtMPVPlayer::handleMpvEvents, Qt::QueuedConnection);

    MPV::Qt::set_wakeup_callback(m_mpv, wakeup, this);

    if (MPV::Qt::initialize(m_mpv) < 0) {
        qFatal("Failed to initialize mpv.");
    }

    connect(this, &QtMPVPlayer::onUpdate, this, &QtMPVPlayer::doUpdate, Qt::QueuedConnection);
}

QtMPVPlayer::~QtMPVPlayer()
{
    // Only initialized if something got drawn
    if (m_mpv_gl) {
        MPV::Qt::render_context_free(m_mpv_gl);
    }
    MPV::Qt::terminate_destroy(m_mpv);
    if (!m_livePreview) {
        qDebug() << "Player destroyed.";
    }
}

void QtMPVPlayer::on_update(void *ctx)
{
    Q_ASSERT(ctx);
    if (!ctx) {
        return;
    }
    Q_EMIT static_cast<QtMPVPlayer *>(ctx)->onUpdate();
}

bool QtMPVPlayer::backendAvailable() const
{
    return MPV::Qt::libmpvAvailability();
}

QString QtMPVPlayer::backendName() const
{
    return QStringLiteral("mpv");
}

QString QtMPVPlayer::backendVersion() const
{
    return QStringLiteral("1.0.0.0");
}

QString QtMPVPlayer::backendDescription() const
{
    return tr("mpv backend.");
}

QString QtMPVPlayer::backendVendor() const
{
    return QStringLiteral("mpv developers");
}

QString QtMPVPlayer::backendCopyright() const
{
    return QStringLiteral("GPLv3");
}

QUrl QtMPVPlayer::backendWebsite() const
{
    return QStringLiteral("https://mpv.io/");
}

// Connected to onUpdate() signal makes sure it runs on the GUI thread
void QtMPVPlayer::doUpdate()
{
    update();
}

void QtMPVPlayer::processMpvLogMessage(void *event)
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
    switch (e->log_level) {
    case MPV_LOG_LEVEL_V:
    case MPV_LOG_LEVEL_DEBUG:
    case MPV_LOG_LEVEL_TRACE:
        qDebug() << message;
        break;
    case MPV_LOG_LEVEL_WARN:
        qWarning() << message;
        break;
    case MPV_LOG_LEVEL_ERROR:
        qCritical() << message;
        break;
    case MPV_LOG_LEVEL_FATAL:
        // qFatal() doesn't support the "<<" operator.
        qFatal("%s", e->text);
        break;
    case MPV_LOG_LEVEL_INFO:
        qInfo() << message;
        break;
    default:
        qDebug() << message;
        break;
    }
}

void QtMPVPlayer::processMpvPropertyChange(void *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    const auto e = static_cast<mpv_event_property *>(event);
    const QString name = QString::fromUtf8(e->name);
    if (!propertyBlackList.contains(name) && !m_livePreview) {
        qDebug() << name << "-->" << mpvGetProperty(name, true);
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

bool QtMPVPlayer::isLoaded() const
{
    return ((m_mediaStatus != MediaStatus::Invalid)
            && (m_mediaStatus != MediaStatus::NoMedia)
            && (m_mediaStatus != MediaStatus::Unloaded)
            && (m_mediaStatus != MediaStatus::Loading));
}

bool QtMPVPlayer::isPlaying() const
{
    return playbackState() == PlaybackState::Playing;
}

bool QtMPVPlayer::isPaused() const
{
    return playbackState() == PlaybackState::Paused;
}

bool QtMPVPlayer::isStopped() const
{
    return playbackState() == PlaybackState::Stopped;
}

void QtMPVPlayer::setMediaStatus(const MediaStatus mediaStatus)
{
    if (m_mediaStatus == mediaStatus) {
        return;
    }
    m_mediaStatus = mediaStatus;
    Q_EMIT mediaStatusChanged();
}

void QtMPVPlayer::videoReconfig()
{
    Q_EMIT videoSizeChanged();
}

void QtMPVPlayer::audioReconfig()
{
    // TODO
}

void QtMPVPlayer::playbackStateChangeEvent()
{
    if (isPlaying()) {
        Q_EMIT playing();
    }
    if (isPaused()) {
        Q_EMIT paused();
    }
    if (isStopped()) {
        Q_EMIT stopped();
    }
    Q_EMIT playbackStateChanged();
}

bool QtMPVPlayer::mpvSendCommand(const QVariant &arguments)
{
    if (!arguments.isValid()) {
        return false;
    }
    if (!m_livePreview) {
        qDebug() << "Command:" << arguments;
    }
    const int errorCode = MPV::Qt::get_error(MPV::Qt::command(m_mpv, arguments));
    if ((errorCode < 0) && !m_livePreview) {
        qWarning() << "Failed to send command" << arguments << ':' << MPV::Qt::error_string(errorCode);
    }
    return (errorCode >= 0);
}

bool QtMPVPlayer::mpvSetProperty(const QString &name, const QVariant &value)
{
    if (name.isEmpty() || !value.isValid()) {
        return false;
    }
    if (!m_livePreview) {
        qDebug() << name << "-->" << value;
    }
    const int errorCode = MPV::Qt::set_property(m_mpv, name, value);
    if ((errorCode < 0) && !m_livePreview) {
        qWarning() << "Failed to change property" << name << "to" << value << ':' << MPV::Qt::error_string(errorCode);
    }
    return (errorCode >= 0);
}

QVariant QtMPVPlayer::mpvGetProperty(const QString &name, const bool silent, bool *ok) const
{
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
            qWarning() << "Failed to query property" << name << ':' << MPV::Qt::error_string(errorCode);
        }
    } else {
        if (ok) {
            *ok = true;
        }
        /*if ((name != "time-pos") && (name != "duration")) {
            qDebug() << "Querying a property from mpv:" << name << "result:" << result;
        }*/
    }
    return result;
}

bool QtMPVPlayer::mpvObserveProperty(const QString &name)
{
    if (name.isEmpty()) {
        return false;
    }
    const int errorCode = MPV::Qt::observe_property(m_mpv, name, 0);
    if ((errorCode < 0) && !m_livePreview) {
        qWarning() << "Failed to observe property" << name << ':' << MPV::Qt::error_string(errorCode);
    }
    return (errorCode >= 0);
}

QUrl QtMPVPlayer::source() const
{
    return m_source;
}

QString QtMPVPlayer::fileName() const
{
    return isStopped() ? QString{} : mpvGetProperty(QStringLiteral("filename")).toString();
}

QSizeF QtMPVPlayer::videoSize() const
{
    if (isStopped()) {
        return {};
    }
    return {mpvGetProperty(QStringLiteral("video-out-params/dw")).toReal(),
            mpvGetProperty(QStringLiteral("video-out-params/dh")).toReal()};
}

QtMPVPlayer::PlaybackState QtMPVPlayer::playbackState() const
{
    const bool stopped = mpvGetProperty(QStringLiteral("idle-active")).toBool();
    const bool paused = mpvGetProperty(QStringLiteral("pause")).toBool();
    return stopped ? PlaybackState::Stopped : (paused ? PlaybackState::Paused : PlaybackState::Playing);
}

QtMPVPlayer::MediaStatus QtMPVPlayer::mediaStatus() const
{
    return m_mediaStatus;
}

QtMPVPlayer::LogLevel QtMPVPlayer::logLevel() const
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

qint64 QtMPVPlayer::duration() const
{
    return isStopped() ? 0 : mpvGetProperty(QStringLiteral("duration")).toLongLong();
}

qint64 QtMPVPlayer::position() const
{
    return isStopped() ? 0 : mpvGetProperty(QStringLiteral("time-pos")).toLongLong();
}

qreal QtMPVPlayer::volume() const
{
    return (mpvGetProperty(QStringLiteral("ao-volume")).toReal() / 100.0);
}

bool QtMPVPlayer::mute() const
{
    return mpvGetProperty(QStringLiteral("ao-mute")).toBool();
}

bool QtMPVPlayer::seekable() const
{
    return isStopped() ? false : mpvGetProperty(QStringLiteral("seekable")).toBool();
}

bool QtMPVPlayer::hardwareDecoding() const
{
    // Querying "hwdec" itself will return empty string.
    const QString hwdec = mpvGetProperty(QStringLiteral("hwdec-current")).toString();
    return (!hwdec.isEmpty() && (hwdec != QStringLiteral("no")) && (hwdec != QStringLiteral("off")));
}

qreal QtMPVPlayer::aspectRatio() const
{
    return isStopped() ? (16.0 / 9.0) : mpvGetProperty(QStringLiteral("video-out-params/aspect")).toReal();
}

qreal QtMPVPlayer::playbackRate() const
{
    return mpvGetProperty(QStringLiteral("speed")).toReal();
}

QString QtMPVPlayer::snapshotFormat() const
{
    return mpvGetProperty(QStringLiteral("screenshot-format")).toString();
}

QString QtMPVPlayer::snapshotTemplate() const
{
    return mpvGetProperty(QStringLiteral("screenshot-template")).toString();
}

QUrl QtMPVPlayer::snapshotDirectory() const
{
    return mpvGetProperty(QStringLiteral("screenshot-directory")).toString();
}

QString QtMPVPlayer::filePath() const
{
    return isStopped() ? QString{} : QDir::toNativeSeparators(mpvGetProperty(QStringLiteral("path")).toString());
}

QtMPVPlayer::MediaTracks QtMPVPlayer::mediaTracks() const
{
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
        if ((type != QStringLiteral("video")) && (type != QStringLiteral("audio")) && (type != QStringLiteral("sub"))) {
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
        if (type == QStringLiteral("video")) {
            info.insert(QStringLiteral("albumart"), trackInfo.value(QStringLiteral("albumart")));
            info.insert(QStringLiteral("demux-w"), trackInfo.value(QStringLiteral("demux-w")));
            info.insert(QStringLiteral("demux-h"), trackInfo.value(QStringLiteral("demux-h")));
            info.insert(QStringLiteral("demux-fps"), trackInfo.value(QStringLiteral("demux-fps")));
            result.video.append(info);
        } else if (type == QStringLiteral("audio")) {
            info.insert(QStringLiteral("demux-channel-count"), trackInfo.value(QStringLiteral("demux-channel-count")));
            info.insert(QStringLiteral("demux-channels"), trackInfo.value(QStringLiteral("demux-channels")));
            info.insert(QStringLiteral("demux-samplerate"), trackInfo.value(QStringLiteral("demux-samplerate")));
            result.audio.append(info);
        } else if (type == QStringLiteral("sub")) {
            result.sub.append(info);
        }
    }
    return result;
}

QtMPVPlayer::Chapters QtMPVPlayer::chapters() const
{
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
        info.startTime = qRound(chapterInfo.value(QStringLiteral("time")).toReal());
        result.append(info);
    }
    return result;
}

QtMPVPlayer::MetaData QtMPVPlayer::metaData() const
{
    const QVariantMap md = mpvGetProperty(QStringLiteral("metadata")).toMap();
    if (md.isEmpty()) {
        return {};
    }
    MetaData result = {};
    auto iterator = md.cbegin();
    while (iterator != md.cend()) {
        result.insert(iterator.key(), iterator.value());
        ++iterator;
    }
    return result;
}

bool QtMPVPlayer::livePreview() const
{
    return m_livePreview;
}

void QtMPVPlayer::play()
{
    if (!m_source.isValid() || m_livePreview) {
        return;
    }
    mpvSetProperty(QStringLiteral("pause"), false);
    playbackStateChangeEvent();
}

void QtMPVPlayer::pause()
{
    if (!isPlaying()) {
        return;
    }
    mpvSetProperty(QStringLiteral("pause"), true);
    playbackStateChangeEvent();
}

void QtMPVPlayer::stop()
{
    if (isStopped()) {
        return;
    }
    mpvSendCommand(QVariantList{QStringLiteral("stop")});
    playbackStateChangeEvent();
}

void QtMPVPlayer::seek(const qint64 value)
{
    if (isStopped() || (position() == value)) {
        return;
    }
    mpvSendCommand(QVariantList{QStringLiteral("seek"), value, QStringLiteral("absolute")});
}

void QtMPVPlayer::snapshot()
{
    if (isStopped()) {
        return;
    }
    // Replace "subtitles" with "video" if you don't want to include subtitles
    // when screenshotting.
    mpvSendCommand(QVariantList{QStringLiteral("screenshot"), QStringLiteral("subtitles")});
}

void QtMPVPlayer::setSource(const QUrl &value)
{
    if (value.isEmpty()) {
        stop();
        m_source.clear();
        Q_EMIT sourceChanged();
        return;
    }
    if (!value.isValid() || (value == m_source)) {
        return;
    }
    const bool result = mpvSendCommand(QVariantList{QStringLiteral("loadfile"), value.isLocalFile() ? QDir::toNativeSeparators(value.toLocalFile()) : value.url()});
    if (result) {
        if (m_livePreview || !m_autoStart) {
            mpvSetProperty(QStringLiteral("pause"), true);
        }
        m_source = value;
        Q_EMIT sourceChanged();
    }
}

void QtMPVPlayer::setMute(const bool value)
{
    if (mute() == value) {
        return;
    }
    mpvSetProperty(QStringLiteral("ao-mute"), value);
}

void QtMPVPlayer::setPlaybackState(const QtMPVPlayer::PlaybackState value)
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

void QtMPVPlayer::setLogLevel(const LogLevel value)
{
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
    const int errorCode = MPV::Qt::request_log_messages(m_mpv, level);
    if (result1 && result2 && (errorCode >= 0)) {
        Q_EMIT logLevelChanged();
    } else {
        if (!m_livePreview) {
            qWarning() << "Failed to change log level to" << level << ':' << MPV::Qt::error_string(errorCode);
        }
    }
}

void QtMPVPlayer::setPosition(const qint64 value)
{
    if (isStopped() || (position() == value)) {
        return;
    }
    seek(value);
}

void QtMPVPlayer::setVolume(const qreal value)
{
    if (volume() == value) {
        return;
    }
    mpvSetProperty(QStringLiteral("ao-volume"), qRound(value * 100.0));
}

void QtMPVPlayer::setHardwareDecoding(const bool value)
{
    if (hardwareDecoding() == value) {
        return;
    }
    mpvSetProperty(QStringLiteral("hwdec"), QStringLiteral("auto-safe"));
}

bool QtMPVPlayer::autoStart() const
{
    return m_autoStart;
}

void QtMPVPlayer::setAutoStart(const bool value)
{
    if (m_autoStart == value) {
        return;
    }
    m_autoStart = value;
    Q_EMIT autoStartChanged();
}

void QtMPVPlayer::setAspectRatio(const qreal value)
{
    if (aspectRatio() == value) {
        return;
    }
    mpvSetProperty(QStringLiteral("video-aspect-override"), value);
}

void QtMPVPlayer::setPlaybackRate(const qreal value)
{
    if (playbackRate() == value) {
        return;
    }
    mpvSetProperty(QStringLiteral("speed"), value);
}

void QtMPVPlayer::setSnapshotFormat(const QString &value)
{
    if (value.isEmpty() || (snapshotFormat() == value)) {
        return;
    }
    mpvSetProperty(QStringLiteral("screenshot-format"), value);
}

void QtMPVPlayer::setSnapshotTemplate(const QString &value)
{
    if (value.isEmpty() || (snapshotTemplate() == value)) {
        return;
    }
    mpvSetProperty(QStringLiteral("screenshot-template"), value);
}

void QtMPVPlayer::setSnapshotDirectory(const QUrl &value)
{
    if (!value.isValid() || (snapshotDirectory() == value)) {
        return;
    }
    mpvSetProperty(QStringLiteral("screenshot-directory"), QDir::toNativeSeparators(value.toLocalFile()));
}

void QtMPVPlayer::setLivePreview(const bool value)
{
    if (m_livePreview == value) {
        return;
    }
    if (value) {
        setLogLevel(LogLevel::Off);
        mpvSetProperty(QStringLiteral("pause"), true);
        mpvSetProperty(QStringLiteral("ao-mute"), true);
        mpvSetProperty(QStringLiteral("hr-seek"), QStringLiteral("yes"));
    } else {
        mpvSetProperty(QStringLiteral("hr-seek"), QStringLiteral("default"));
        setLogLevel(LogLevel::Warning); // TODO: back to previous
    }
    m_livePreview = value;
    Q_EMIT livePreviewChanged();
}

QtMPVPlayer::FillMode QtMPVPlayer::fillMode() const
{
    // TODO
    return FillMode::PreserveAspectFit;
}

void QtMPVPlayer::setFillMode(const QtMPVPlayer::FillMode value)
{
    // TODO
    Q_UNUSED(value);
}

void QtMPVPlayer::handleMpvEvents()
{
    // Process all events, until the event queue is empty.
    while (m_mpv) {
        const auto event = MPV::Qt::wait_event(m_mpv, 0.005);
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
            setMediaStatus(MediaStatus::Loading);
            break;
        // Notification after playback end (after the file was unloaded).
        // See also mpv_event and mpv_event_end_file.
        case MPV_EVENT_END_FILE:
            setMediaStatus(MediaStatus::End);
            playbackStateChangeEvent();
            break;
        // Notification when the file has been loaded (headers were read
        // etc.), and decoding starts.
        case MPV_EVENT_FILE_LOADED:
            setMediaStatus(MediaStatus::Loaded);
            Q_EMIT loaded();
            playbackStateChangeEvent();
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
            break;
        // There was a discontinuity of some sort (like a seek), and playback
        // was reinitialized. Usually happens after seeking, or ordered chapter
        // segment switches. The main purpose is allowing the client to detect
        // when a seek request is finished.
        case MPV_EVENT_PLAYBACK_RESTART:
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
            qDebug() << MPV::Qt::event_name(event->event_id) << "event received.";
        }
    }
}

// The beauty of using a true QSGNode: no need for complicated cleanup
// arrangements, unlike in other examples like metalunderqml, because the
// scenegraph will handle destroying the node at the appropriate time.
void QtMPVPlayer::invalidateSceneGraph() // Called on the render thread when the scenegraph is invalidated
{
    m_node = nullptr;
}

void QtMPVPlayer::releaseResources() // Called on the gui thread if the item is removed from scene
{
    m_node = nullptr;
}

QSGNode *QtMPVPlayer::updatePaintNode(QSGNode *node, UpdatePaintNodeData *data)
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
void QtMPVPlayer::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
#else
void QtMPVPlayer::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
#endif
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QtMediaPlayer::geometryChange(newGeometry, oldGeometry);
#else
    QtMediaPlayer::geometryChanged(newGeometry, oldGeometry);
#endif
    if (newGeometry.size() != oldGeometry.size()) {
        update();
    }
}

QTMEDIAPLAYER_END_NAMESPACE
