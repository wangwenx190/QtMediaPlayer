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

#include "mdkplayer.h"
#include "mdkbackend.h"
#include "mdkvideotexturenode.h"
#include "mdkqthelper.h"
#include <backendinterface.h>
#include "include/mdk/Player.h"
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qdatetime.h>
#include <QtQuick/qquickwindow.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

[[nodiscard]] extern MDKVideoTextureNode *createNode(MDKPlayer *item);

[[nodiscard]] static inline std::vector<std::string> qStringListToStdStringVector(const QStringList &stringList)
{
    if (stringList.isEmpty()) {
        return {};
    }
    std::vector<std::string> result = {};
    for (auto &&string : qAsConst(stringList)) {
        result.push_back(string.toStdString());
    }
    return result;
}

[[nodiscard]] static inline QString urlToString(const QUrl &value, const bool display = false)
{
    if (!value.isValid()) {
        return {};
    }
    return (value.isLocalFile() ? QDir::toNativeSeparators(value.toLocalFile())
                                : (display ? value.toDisplayString() : value.toString()));
}

[[nodiscard]] static inline MediaStatus mediaStatusFromMDK(const MDK_NS_PREPEND(MediaStatus) ms)
{
    MediaStatus result = {};
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::NoMedia)) {
        result |= MediaStatusFlag::NoMedia;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::Unloaded)) {
        result |= MediaStatusFlag::Unloaded;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::Loading)) {
        result |= MediaStatusFlag::Loading;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::Loaded)) {
        result |= MediaStatusFlag::Loaded;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::Prepared)) {
        result |= MediaStatusFlag::Prepared;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::Stalled)) {
        result |= MediaStatusFlag::Stalled;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::Buffering)) {
        result |= MediaStatusFlag::Buffering;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::Buffered)) {
        result |= MediaStatusFlag::Buffered;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::End)) {
        result |= MediaStatusFlag::End;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::Seeking)) {
        result |= MediaStatusFlag::Seeking;
    }
    if (MDK_NS_PREPEND(test_flag)(ms, MDK_NS_PREPEND(MediaStatus)::Invalid)) {
        result |= MediaStatusFlag::Invalid;
    }
    return result;
}

MDKPlayer::MDKPlayer(QQuickItem *parent) : MediaPlayer(parent)
{
    initialize();
}

MDKPlayer::~MDKPlayer()
{
    deinitialize();
}

void MDKPlayer::initialize()
{
    qCDebug(lcQMPMDK) << "Initializing the MDK backend ...";

    if (!MDK::Qt::isMDKAvailable()) {
        qFatal("MDK is not available.");
    }

    m_player.reset(new MDK_NS_PREPEND(Player));

    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Player created.";
    }

    m_player->setRenderCallback([this](void *){
        QMetaObject::invokeMethod(this, "update");
    });

    // Default to software decoding.
    m_player->setDecoders(MDK_NS_PREPEND(MediaType)::Video, {"FFmpeg"});

    m_snapshotDirectory = QUrl(QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));

    connect(this, &MDKPlayer::sourceChanged, this, &MDKPlayer::fileNameChanged);
    connect(this, &MDKPlayer::sourceChanged, this, &MDKPlayer::filePathChanged);

    initMdkHandlers();

    m_timer.setTimerType(Qt::CoarseTimer);
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, [this](){
        if (isPlaying()) {
            const qint64 currentPosition = position();
            if (currentPosition != m_lastPosition) {
                m_lastPosition = currentPosition;
                Q_EMIT positionChanged();
            }
        }
    });
    m_timer.start();

    connect(this, &MDKPlayer::rendererReadyChanged, this, [this](){
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

void MDKPlayer::deinitialize()
{
    if (!isStopped()) {
        stop();
    }
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Player destroyed.";
    }
}

QString MDKPlayer::backendName() const
{
    return metaData_mdk().value(kName).toString();
}

QString MDKPlayer::backendVersion() const
{
    return metaData_mdk().value(kVersion).toString();
}

QString MDKPlayer::backendAuthors() const
{
    return metaData_mdk().value(kAuthors).toString();
}

QString MDKPlayer::backendCopyright() const
{
    return metaData_mdk().value(kCopyright).toString();
}

QString MDKPlayer::backendLicenses() const
{
    return metaData_mdk().value(kLicenses).toString();
}

QString MDKPlayer::backendHomepage() const
{
    return metaData_mdk().value(kHomepage).toString();
}

QString MDKPlayer::ffmpegVersion() const
{
    return metaData_mdk().value(kFFmpegVersion).toString();
}

QString MDKPlayer::ffmpegConfiguration() const
{
    return metaData_mdk().value(kFFmpegConfiguration).toString();
}

// The beauty of using a true QSGNode: no need for complicated cleanup
// arrangements, unlike in other examples like metalunderqml, because the
// scenegraph will handle destroying the node at the appropriate time.
void MDKPlayer::invalidateSceneGraph() // Called on the render thread when the scenegraph is invalidated.
{
    m_node = nullptr;
}

void MDKPlayer::setRendererReady(const bool value)
{
    if (m_rendererReady == value) {
        return;
    }
    m_rendererReady = value;
    Q_EMIT rendererReadyChanged();
}

void MDKPlayer::releaseResources() // Called on the gui thread if the item is removed from scene.
{
    m_node = nullptr;
}

QSGNode *MDKPlayer::updatePaintNode(QSGNode *node, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    auto n = static_cast<MDKVideoTextureNode *>(node);
    if (!n && ((width() <= 0) || (height() <= 0))) {
        return nullptr;
    }
    if (!n) {
        m_node = createNode(this);
        n = m_node;
    }
    m_node->sync();
    window()->update(); // Ensure getting to beforeRendering() at some point.
    return n;
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void MDKPlayer::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
#else
void MDKPlayer::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
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

QUrl MDKPlayer::source() const
{
    if (!m_player->url()) {
        return {};
    }
    return QUrl::fromUserInput(QString::fromUtf8(m_player->url()),
                               QCoreApplication::applicationDirPath(), QUrl::AssumeLocalFile);
}

void MDKPlayer::setSource(const QUrl &value)
{
    if (!m_rendererReady) {
        m_cachedUrl = value;
        return;
    }
    const auto realStop = [this]() -> void {
        m_player->setMedia(nullptr);
        m_player->setNextMedia(nullptr);
        m_player->set(MDK_NS_PREPEND(PlaybackState)::Stopped);
        m_player->waitFor(MDK_NS_PREPEND(PlaybackState)::Stopped);
    };
    if (value.isEmpty()) {
        qCDebug(lcQMPMDK) << "Empty source is set, playback stopped.";
        realStop();
        return;
    }
    if (!value.isValid()) {
        qCWarning(lcQMPMDK) << "The given URL" << value << "is invalid.";
        return;
    }
    if (QString::compare(value.scheme(), QStringLiteral("qrc"), Qt::CaseInsensitive) == 0) {
        qCWarning(lcQMPMDK) << "Currently embeded resource is not supported.";
        return;
    }
    const QString filename = value.fileName();
    if (filename.isEmpty()) {
        qCWarning(lcQMPMDK) << "The source url" << value << "doesn't contain a filename.";
        return;
    }
    if (!isMediaFile(filename)) {
        qCWarning(lcQMPMDK) << "The source url" << value << "doesn't seem to be a multimedia file.";
        return;
    }
    if (value == source()) {
        if (isStopped() && !m_livePreview) {
            m_player->set(MDK_NS_PREPEND(PlaybackState)::Playing);
        }
        return;
    }
    realStop();
    m_player->setMedia(qUtf8Printable(urlToString(value)));
    Q_EMIT sourceChanged();
    // It's necessary to call "prepare()", otherwise we'll get no picture.
    m_player->prepare();
    if (m_autoStart && !m_livePreview) {
        m_player->set(MDK_NS_PREPEND(PlaybackState)::Playing);
    }
}

QString MDKPlayer::fileName() const
{
    const QUrl url = source();
    return (url.isValid() ? url.fileName() : QString{});
}

QString MDKPlayer::filePath() const
{
    const QUrl url = source();
    return (url.isValid() ? urlToString(url, true) : QString{});
}

qint64 MDKPlayer::position() const
{
    return (isLoaded() ? m_player->position() : 0);
}

void MDKPlayer::setPosition(const qint64 value)
{
    seek(value);
}

qint64 MDKPlayer::duration() const
{
    if (!isLoaded()) {
        return 0;
    }
    const auto &mi = m_player->mediaInfo();
    return mi.duration;
}

QSizeF MDKPlayer::videoSize() const
{
    if (!isLoaded()) {
        return {};
    }
    const auto &mi = m_player->mediaInfo();
    const auto &vs = mi.video;
    if (vs.empty()) {
        return {};
    }
    const auto &vsf = vs.at(0);
    return {static_cast<qreal>(vsf.codec.width), static_cast<qreal>(vsf.codec.height)};
}

qreal MDKPlayer::volume() const
{
    return m_volume;
}

void MDKPlayer::setVolume(const qreal value)
{
    if (qFuzzyCompare(value, m_volume)) {
        return;
    }
    if (value < 0.0) {
        qCWarning(lcQMPMDK) << "The minimum volume is 0, however, the user is trying to change it to" << value;
        return;
    }
    if (value > 1.0) {
        qCWarning(lcQMPMDK) << "The maximum volume is 1.0, however, the user is trying to change it to" << value
                   << ". It's allowed but it may cause damaged sound.";
    }
    m_volume = value;
    m_player->setVolume(m_volume);
    Q_EMIT volumeChanged();
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Volume -->" << m_volume;
    }
}

bool MDKPlayer::mute() const
{
    return m_mute;
}

void MDKPlayer::setMute(const bool value)
{
    if (value == m_mute) {
        return;
    }
    m_mute = value;
    m_player->setMute(m_mute);
    Q_EMIT muteChanged();
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Mute -->" << m_mute;
    }
}

bool MDKPlayer::seekable() const
{
    // Local files are always seekable, in theory.
    return (isLoaded() && source().isLocalFile());
}

PlaybackState MDKPlayer::playbackState() const
{
    switch (m_player->state()) {
    case MDK_NS_PREPEND(PlaybackState)::Playing:
        return PlaybackState::Playing;
    case MDK_NS_PREPEND(PlaybackState)::Paused:
        return PlaybackState::Paused;
    case MDK_NS_PREPEND(PlaybackState)::Stopped:
        return PlaybackState::Stopped;
    }
    return PlaybackState::Stopped;
}

void MDKPlayer::setPlaybackState(const PlaybackState value)
{
    if (value == playbackState()) {
        return;
    }
    switch (value) {
    case PlaybackState::Playing: {
        if (!m_livePreview) {
            m_player->set(MDK_NS_PREPEND(PlaybackState)::Playing);
        }
    } break;
    case PlaybackState::Paused:
        m_player->set(MDK_NS_PREPEND(PlaybackState)::Paused);
        break;
    case PlaybackState::Stopped:
        m_player->set(MDK_NS_PREPEND(PlaybackState)::Stopped);
        break;
    }
}

MediaStatus MDKPlayer::mediaStatus() const
{
    return m_mediaStatus;
}

LogLevel MDKPlayer::logLevel() const
{
    switch (static_cast<MDK_NS_PREPEND(LogLevel)>(MDK_logLevel())) {
    case MDK_NS_PREPEND(LogLevel)::Off:
        return LogLevel::Off;
    case MDK_NS_PREPEND(LogLevel)::Debug:
        return LogLevel::Debug;
    case MDK_NS_PREPEND(LogLevel)::Warning:
        return LogLevel::Warning;
    case MDK_NS_PREPEND(LogLevel)::Error:
        return LogLevel::Critical;
    case MDK_NS_PREPEND(LogLevel)::Info:
        return LogLevel::Info;
    default:
        return LogLevel::Debug;
    }
}

void MDKPlayer::setLogLevel(const LogLevel value)
{
    MDK_NS_PREPEND(LogLevel) logLv = MDK_NS_PREPEND(LogLevel)::Debug;
    switch (value) {
    case LogLevel::Off:
        logLv = MDK_NS_PREPEND(LogLevel)::Off;
        break;
    case LogLevel::Debug:
        logLv = MDK_NS_PREPEND(LogLevel)::Debug;
        break;
    case LogLevel::Warning:
        logLv = MDK_NS_PREPEND(LogLevel)::Warning;
        break;
    case LogLevel::Critical:
    case LogLevel::Fatal:
        logLv = MDK_NS_PREPEND(LogLevel)::Error;
        break;
    case LogLevel::Info:
        logLv = MDK_NS_PREPEND(LogLevel)::Info;
        break;
    }
    MDK_NS_PREPEND(SetGlobalOption)("logLevel", logLv);
    Q_EMIT logLevelChanged();
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Log level -->" << value;
    }
}

qreal MDKPlayer::playbackRate() const
{
    return static_cast<qreal>(m_player->playbackRate());
}

void MDKPlayer::setPlaybackRate(const qreal value)
{
    if (qFuzzyCompare(value, playbackRate())) {
        return;
    }
    if (value <= 0.0) {
        qCWarning(lcQMPMDK) << "The user is trying to change the playback rate to" << value << ", which is not allowed.";
        return;
    }
    m_player->setPlaybackRate(value);
    Q_EMIT playbackRateChanged();
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Playback rate -->" << value;
    }
}

qreal MDKPlayer::aspectRatio() const
{
    const QSizeF vs = videoSize();
    if (vs.isEmpty()) {
        return (16.0 / 9.0);
    }
    return (vs.width() / vs.height());
}

void MDKPlayer::setAspectRatio(const qreal value)
{
    if (qFuzzyCompare(value, aspectRatio())) {
        return;
    }
    if (value <= 0.0) {
        qCWarning(lcQMPMDK) << "The user is trying to change the aspect ratio to" << value << ", which is not allowed.";
        return;
    }
    m_player->setAspectRatio(value);
    Q_EMIT aspectRatioChanged();
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Aspect ratio -->" << value;
    }
}

QUrl MDKPlayer::snapshotDirectory() const
{
    return m_snapshotDirectory;
}

void MDKPlayer::setSnapshotDirectory(const QUrl &value)
{
    if (value.isEmpty() || (value == snapshotDirectory())) {
        return;
    }
    m_snapshotDirectory = value;
    Q_EMIT snapshotDirectoryChanged();
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Snapshot directory -->" << m_snapshotDirectory;
    }
}

QString MDKPlayer::snapshotFormat() const
{
    return m_snapshotFormat;
}

void MDKPlayer::setSnapshotFormat(const QString &value)
{
    if (value.isEmpty() || (value == m_snapshotFormat)) {
        return;
    }
    m_snapshotFormat = value.toLower();
    Q_EMIT snapshotFormatChanged();
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Snapshot format -->" << m_snapshotFormat;
    }
}

QString MDKPlayer::snapshotTemplate() const
{
    return m_snapshotTemplate;
}

void MDKPlayer::setSnapshotTemplate(const QString &value)
{
    if (value.isEmpty() || (value == m_snapshotTemplate)) {
        return;
    }
    m_snapshotTemplate = value;
    Q_EMIT snapshotTemplateChanged();
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Snapshot template -->" << m_snapshotTemplate;
    }
}

bool MDKPlayer::hardwareDecoding() const
{
    return m_hardwareDecoding;
}

void MDKPlayer::setHardwareDecoding(const bool value)
{
    const auto setVideoDecoders = [this](const QStringList &decoders) -> void
    {
        const QStringList videoDecoders = decoders.isEmpty() ? QStringList{QStringLiteral("FFmpeg")} : decoders;
        m_player->setDecoders(MDK_NS_PREPEND(MediaType)::Video, qStringListToStdStringVector(videoDecoders));
        if (!m_livePreview) {
            qCDebug(lcQMPMDK) << "Video decoders -->" << videoDecoders;
        }
    };
    // The order is important. Only FFmpeg is software decoding.
    static const QStringList defaultVideoDecoders =
    {
#ifdef Q_OS_WINDOWS
        QStringLiteral("MFT:d3d=11"),
        QStringLiteral("MFT:d3d=9"),
        QStringLiteral("MFT"),
        QStringLiteral("D3D11"),
        QStringLiteral("DXVA"),
        QStringLiteral("CUDA"),
        QStringLiteral("NVDEC"),
#elif (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
        QStringLiteral("VAAPI"),
        QStringLiteral("VDPAU"),
        QStringLiteral("CUDA"),
        QStringLiteral("NVDEC"),
#elif (defined(Q_OS_MACOS) || defined(Q_OS_IOS))
        QStringLiteral("VT"),
        QStringLiteral("VideoToolbox"),
#elif defined(Q_OS_ANDROID)
        QStringLiteral("AMediaCodec"),
#endif
        QStringLiteral("FFmpeg")
    };
    if (m_hardwareDecoding != value) {
        m_hardwareDecoding = value;
        if (m_hardwareDecoding) {
            static bool warningOnce = false;
            if (!warningOnce) {
                warningOnce = true;
                qCWarning(lcQMPMDK).noquote() << hardwareDecodingWarningText;
            }
            setVideoDecoders(defaultVideoDecoders);
        } else {
            setVideoDecoders({QStringLiteral("FFmpeg")});
        }
        Q_EMIT hardwareDecodingChanged();
        if (!m_livePreview) {
            qCDebug(lcQMPMDK) << "Hardware decoding -->" << m_hardwareDecoding;
        }
    }
}

bool MDKPlayer::autoStart() const
{
    return m_autoStart;
}

void MDKPlayer::setAutoStart(const bool value)
{
    if (m_autoStart != value) {
        m_autoStart = value;
        Q_EMIT autoStartChanged();
        if (!m_livePreview) {
            qCDebug(lcQMPMDK) << "Auto start -->" << m_autoStart;
        }
    }
}

bool MDKPlayer::livePreview() const
{
    return m_livePreview;
}

void MDKPlayer::setLivePreview(const bool value)
{
    if (m_livePreview != value) {
        m_livePreview = value;
        if (m_livePreview) {
            // We only need static images.
            m_player->set(MDK_NS_PREPEND(PlaybackState)::Paused);
            // We don't want the preview window play sound.
            m_player->setMute(true);
            // Disable media tracks that are not needed.
            m_player->setActiveTracks(MDK_NS_PREPEND(MediaType)::Audio, {});
            m_player->setActiveTracks(MDK_NS_PREPEND(MediaType)::Subtitle, {});
            // Decode as soon as possible when media data received.
            m_player->setBufferRange(0);
            // Prevent player stop playing after EOF is reached.
            m_player->setProperty("continue_at_end", "1");
            // And don't forget to use accurate seek.
        } else {
            // Restore everything to default.
            m_player->setBufferRange(1000);
            m_player->setActiveTracks(MDK_NS_PREPEND(MediaType)::Audio, {m_activeAudioTrack});
            m_player->setActiveTracks(MDK_NS_PREPEND(MediaType)::Subtitle, {m_activeSubtitleTrack});
            m_player->setMute(m_mute);
            m_player->setProperty("continue_at_end", "0");
        }
        Q_EMIT livePreviewChanged();
    }
}

FillMode MDKPlayer::fillMode() const
{
    return m_fillMode;
}

void MDKPlayer::setFillMode(const FillMode value)
{
    if (m_fillMode != value) {
        m_fillMode = value;
        switch (m_fillMode) {
        case FillMode::PreserveAspectFit:
            m_player->setAspectRatio(MDK_NS_PREPEND(KeepAspectRatio));
            break;
        case FillMode::PreserveAspectCrop:
            m_player->setAspectRatio(MDK_NS_PREPEND(KeepAspectRatioCrop));
            break;
        case FillMode::Stretch:
            m_player->setAspectRatio(MDK_NS_PREPEND(IgnoreAspectRatio));
            break;
        }
        Q_EMIT fillModeChanged();
        if (!m_livePreview) {
            qCDebug(lcQMPMDK) << "Fill mode -->" << m_fillMode;
        }
    }
}

Chapters MDKPlayer::chapters() const
{
    if (!isLoaded()) {
        return {};
    }
    const auto &mi = m_player->mediaInfo();
    const auto &cpts = mi.chapters;
    if (cpts.empty()) {
        return {};
    }
    Chapters result = {};
    for (auto &&chapter : qAsConst(cpts)) {
        ChapterInfo info = {};
        info.title = QString::fromStdString(chapter.title);
        info.startTime = chapter.start_time;
        info.endTime = chapter.end_time;
        result.append(info);
    }
    return result;
}

MetaData MDKPlayer::metaData() const
{
    if (!isLoaded()) {
        return {};
    }
    const auto &mi = m_player->mediaInfo();
    const auto &md = mi.metadata;
    if (md.empty()) {
        return {};
    }
    MetaData result = {};
    for (auto &&data : qAsConst(md)) {
        result.insert(QString::fromStdString(data.first), QString::fromStdString(data.second));
    }
    return result;
}

MediaTracks MDKPlayer::mediaTracks() const
{
    if (!isLoaded()) {
        return {};
    }
    const auto &mi = m_player->mediaInfo();
    const auto &vs = mi.video;
    const auto &as = mi.audio;
    MediaTracks result = {};
    if (!vs.empty()) {
        for (auto &&vsi : qAsConst(vs)) {
            QVariantHash info = {};
            info.insert(QStringLiteral("index"), vsi.index);
            info.insert(QStringLiteral("start_time"), qint64(vsi.start_time));
            info.insert(QStringLiteral("duration"), qint64(vsi.duration));
            info.insert(QStringLiteral("frames"), qint64(vsi.frames));
            info.insert(QStringLiteral("rotation"), vsi.rotation);
            info.insert(QStringLiteral("width"), vsi.codec.width);
            info.insert(QStringLiteral("height"), vsi.codec.height);
            info.insert(QStringLiteral("frame_rate"), vsi.codec.frame_rate);
            info.insert(QStringLiteral("bit_rate"), qint64(vsi.codec.bit_rate));
            info.insert(QStringLiteral("codec"), QString::fromUtf8(vsi.codec.codec));
            info.insert(QStringLiteral("format_name"), QString::fromUtf8(vsi.codec.format_name));
            // What about metadata?
            result.video.append(info);
        }
    }
    if (!as.empty()) {
        for (auto &&asi : qAsConst(as)) {
            QVariantHash info = {};
            info.insert(QStringLiteral("index"), asi.index);
            info.insert(QStringLiteral("start_time"), qint64(asi.start_time));
            info.insert(QStringLiteral("duration"), qint64(asi.duration));
            info.insert(QStringLiteral("frames"), qint64(asi.frames));
            info.insert(QStringLiteral("bit_rate"), qint64(asi.codec.bit_rate));
            info.insert(QStringLiteral("frame_rate"), asi.codec.frame_rate);
            info.insert(QStringLiteral("codec"), QString::fromUtf8(asi.codec.codec));
            info.insert(QStringLiteral("channels"), asi.codec.channels);
            info.insert(QStringLiteral("sample_rate"), asi.codec.sample_rate);
            // What about metadata?
            result.audio.append(info);
        }
    }
    // TODO: subtitles
    return result;
}

int MDKPlayer::activeVideoTrack() const
{
    return m_activeVideoTrack;
}

void MDKPlayer::setActiveVideoTrack(const int value)
{
    if (!isLoaded()) {
        qCWarning(lcQMPMDK) << "Setting active track before the media is loaded has no effect."
                            << "Please try again later when the media has been loaded successfully.";
        return;
    }
    int track = value;
    if (track < 0) {
        track = 0;
        qCWarning(lcQMPMDK) << "The minimum track number is zero, "
                               "setting active track to a negative number equals to reset to default track.";
    } else {
        const int totalTrackCount = mediaTracks().video.count();
        if (track >= totalTrackCount) {
            track = totalTrackCount - 1;
            qCWarning(lcQMPMDK) << "Total video track count is" << totalTrackCount
                                << ". Can't set active track to a number greater or equal to it.";
        }
    }
    if (m_activeVideoTrack == track) {
        return;
    }
    m_player->setActiveTracks(MDK_NS_PREPEND(MediaType)::Video, {track});
    m_activeVideoTrack = track;
    Q_EMIT activeVideoTrackChanged();
}

int MDKPlayer::activeAudioTrack() const
{
    return m_activeAudioTrack;
}

void MDKPlayer::setActiveAudioTrack(const int value)
{
    if (!isLoaded()) {
        qCWarning(lcQMPMDK) << "Setting active track before the media is loaded has no effect."
                            << "Please try again later when the media has been loaded successfully.";
        return;
    }
    int track = value;
    if (track < 0) {
        track = 0;
        qCWarning(lcQMPMDK) << "The minimum track number is zero, "
                               "setting active track to a negative number equals to reset to default track.";
    } else {
        const int totalTrackCount = mediaTracks().audio.count();
        if (track >= totalTrackCount) {
            track = totalTrackCount - 1;
            qCWarning(lcQMPMDK) << "Total audio track count is" << totalTrackCount
                                << ". Can't set active track to a number greater or equal to it.";
        }
    }
    if (m_activeAudioTrack == track) {
        return;
    }
    m_player->setActiveTracks(MDK_NS_PREPEND(MediaType)::Audio, {track});
    m_activeAudioTrack = track;
    Q_EMIT activeAudioTrackChanged();
}

int MDKPlayer::activeSubtitleTrack() const
{
    return m_activeSubtitleTrack;
}

void MDKPlayer::setActiveSubtitleTrack(const int value)
{
    if (!isLoaded()) {
        qCWarning(lcQMPMDK) << "Setting active track before the media is loaded has no effect."
                            << "Please try again later when the media has been loaded successfully.";
        return;
    }
    int track = value;
    if (track < 0) {
        track = 0;
        qCWarning(lcQMPMDK) << "The minimum track number is zero, "
                               "setting active track to a negative number equals to reset to default track.";
    } else {
        const int totalTrackCount = mediaTracks().subtitle.count();
        if (track >= totalTrackCount) {
            track = totalTrackCount - 1;
            qCWarning(lcQMPMDK) << "Total subtitle track count is" << totalTrackCount
                                << ". Can't set active track to a number greater or equal to it.";
        }
    }
    if (m_activeSubtitleTrack == track) {
        return;
    }
    m_player->setActiveTracks(MDK_NS_PREPEND(MediaType)::Subtitle, {track});
    m_activeSubtitleTrack = track;
    Q_EMIT activeSubtitleTrackChanged();
}

bool MDKPlayer::rendererReady() const
{
    return m_rendererReady;
}

void MDKPlayer::play()
{
    if (!source().isValid() || m_livePreview) {
        return;
    }
    m_player->set(MDK_NS_PREPEND(PlaybackState)::Playing);
}

void MDKPlayer::pause()
{
    if (!source().isValid()) {
        return;
    }
    m_player->set(MDK_NS_PREPEND(PlaybackState)::Paused);
}

void MDKPlayer::stop()
{
    if (!source().isValid()) {
        return;
    }
    m_player->setMedia(nullptr);
    m_player->setNextMedia(nullptr);
    m_player->set(MDK_NS_PREPEND(PlaybackState)::Stopped);
}

void MDKPlayer::seek(const qint64 value)
{
    if (!isLoaded() || (value == position())) {
        return;
    }
    const auto &mi = m_player->mediaInfo();
    if (value < mi.start_time) {
        qCWarning(lcQMPMDK) << "Media start time is" << mi.start_time
                            << ", however, the user is trying to seek to" << value;
        return;
    }
    if (value > mi.duration) {
        qCWarning(lcQMPMDK) << "Media duration is" << mi.duration
                            << ", however, the user is trying to seek to" << value;
        return;
    }
    // We have to seek accurately when we are in live preview mode.
    m_player->seek(value, m_livePreview ? MDK_NS_PREPEND(SeekFlag)::FromStart : MDK_NS_PREPEND(SeekFlag)::Default);
    // In case the playback is paused.
    Q_EMIT positionChanged();
    if (!m_livePreview) {
        qCDebug(lcQMPMDK) << "Seek -->" << value;
    }
}

void MDKPlayer::snapshot()
{
    if (!isLoaded()) {
        return;
    }
    MDK_NS_PREPEND(Player)::SnapshotRequest snapshotRequest = {};
    m_player->snapshot(&snapshotRequest, [this](MDK_NS_PREPEND(Player)::SnapshotRequest *ret, qreal frameTime) {
        Q_UNUSED(ret);
        QString path = m_snapshotTemplate;
        const QString completeBaseName = QFileInfo(filePath()).completeBaseName();
        if (path.isEmpty()) {
            path = completeBaseName;
        } else {
            path.replace(QStringLiteral("${filename}"), completeBaseName, Qt::CaseInsensitive);
            const QDateTime currentDateTime = QDateTime::currentDateTime();
            path.replace(QStringLiteral("${date}"), currentDateTime.toString(QStringLiteral("yyyy.MM.dd")), Qt::CaseInsensitive);
            path.replace(QStringLiteral("${time}"), currentDateTime.toString(QStringLiteral("hh.mm.ss.zzz")), Qt::CaseInsensitive);
            path.replace(QStringLiteral("${datetime}"), currentDateTime.toString(QStringLiteral("yyyy.MM.dd.hh.mm.ss.zzz")), Qt::CaseInsensitive);
            path.replace(QStringLiteral("${frametime}"), QString::number(frameTime), Qt::CaseInsensitive);
            path.replace(QStringLiteral("${position}"), QString::number(position()), Qt::CaseInsensitive);
            path.replace(QStringLiteral("${duration}"), QString::number(duration()), Qt::CaseInsensitive);
            const auto &md = metaData();
            path.replace(QStringLiteral("${title}"), md.value(QStringLiteral("title")).toString(), Qt::CaseInsensitive);
            path.replace(QStringLiteral("${author}"), md.value(QStringLiteral("author")).toString(), Qt::CaseInsensitive);
            path.replace(QStringLiteral("${artist}"), md.value(QStringLiteral("artist")).toString(), Qt::CaseInsensitive);
            path.replace(QStringLiteral("${album}"), md.value(QStringLiteral("album")).toString(), Qt::CaseInsensitive);
        }
        path.append(u'.');
        if (m_snapshotFormat.isEmpty()) {
            path.append(QStringLiteral("png"));
        } else {
            path.append(m_snapshotFormat);
        }
        const QString dirPath = QDir::toNativeSeparators(m_snapshotDirectory.toLocalFile());
        if (!dirPath.endsWith(u'/') && !dirPath.endsWith(u'\\')) {
            path.prepend(QDir::separator());
        }
        if (m_snapshotDirectory.isEmpty()) {
            path.prepend(QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));
        } else {
            path.prepend(dirPath);
        }
        if (!m_livePreview) {
            qCDebug(lcQMPMDK) << "Taking snapshot -->" << path;
        }
        return qstrdup(qUtf8Printable(path));
    });
}

void MDKPlayer::initMdkHandlers()
{
    MDK_NS_PREPEND(setLogHandler)([this](MDK_NS_PREPEND(LogLevel) level, const char *msg) {
        if (m_livePreview) {
            // We don't need log messages from the preview player.
            return;
        }
        if (!msg) {
            return;
        }
        const QString text = QString::fromUtf8(msg).trimmed();
        if (text.isEmpty()) {
            return;
        }
        switch (level) {
        case MDK_NS_PREPEND(LogLevel)::Info:
            qCInfo(lcQMPMDK).noquote() << text;
            break;
        case MDK_NS_PREPEND(LogLevel)::All:
        case MDK_NS_PREPEND(LogLevel)::Debug:
            qCDebug(lcQMPMDK).noquote() << text;
            break;
        case MDK_NS_PREPEND(LogLevel)::Warning:
            qCWarning(lcQMPMDK).noquote() << text;
            break;
        case MDK_NS_PREPEND(LogLevel)::Error:
            qCCritical(lcQMPMDK).noquote() << text;
            break;
        default:
            break;
        }
    });
    m_player->currentMediaChanged([this](){
        const QUrl url = source();
        if (!url.isValid()) {
            return;
        }
        if (!m_livePreview) {
            qCDebug(lcQMPMDK) << "Current media source -->" << urlToString(url, true);
        }
        Q_EMIT sourceChanged();
    });
    m_player->onMediaStatusChanged([this](MDK_NS_PREPEND(MediaStatus) ms) {
        m_mediaStatus = mediaStatusFromMDK(ms);
        Q_EMIT mediaStatusChanged();
        if ((m_mediaStatus & MediaStatusFlag::Prepared) && !m_loaded) {
            m_loaded = true;
            Q_EMIT loaded();
            Q_EMIT videoSizeChanged();
            resetInternalData();
        }
        if (!m_livePreview) {
            qCDebug(lcQMPMDK) << "Current media status -->" << m_mediaStatus;
        }
        return true;
    });
    m_player->onEvent([this](const MDK_NS_PREPEND(MediaEvent) &me) {
        if (!m_livePreview) {
            qCDebug(lcQMPMDK) << "MDK event:" << me.category.data() << me.detail.data();
        }
        return false;
    });
    m_player->onLoop([this](int count) {
        if (!m_livePreview) {
            qCDebug(lcQMPMDK) << "loop:" << count;
        }
        return false;
    });
    m_player->onStateChanged([this](MDK_NS_PREPEND(PlaybackState) pbs) {
        Q_EMIT playbackStateChanged();
        if (pbs == MDK_NS_PREPEND(PlaybackState)::Playing) {
            Q_EMIT playing();
            if (!m_livePreview) {
                qCDebug(lcQMPMDK) << "Playing.";
            }
        }
        if (pbs == MDK_NS_PREPEND(PlaybackState)::Paused) {
            Q_EMIT paused();
            if (!m_livePreview) {
                qCDebug(lcQMPMDK) << "Paused.";
            }
        }
        if (pbs == MDK_NS_PREPEND(PlaybackState)::Stopped) {
            const QUrl url = source();
            const qint64 pos = m_lastPosition;
            m_player->setMedia(nullptr);
            m_player->setNextMedia(nullptr);
            m_loaded = false;
            m_mediaStatus = {};
            Q_EMIT stopped();
            Q_EMIT stoppedWithPosition(url, pos);
            Q_EMIT sourceChanged();
            resetInternalData();
            if (!m_livePreview) {
                qCDebug(lcQMPMDK) << "Stopped.";
            }
        }
    });
}

bool MDKPlayer::isLoaded() const
{
    return m_loaded;
}

bool MDKPlayer::isPlaying() const
{
    return (m_player->state() == MDK_NS_PREPEND(PlaybackState)::Playing);
}

bool MDKPlayer::isPaused() const
{
    return (m_player->state() == MDK_NS_PREPEND(PlaybackState)::Paused);
}

bool MDKPlayer::isStopped() const
{
    return (m_player->state() == MDK_NS_PREPEND(PlaybackState)::Stopped);
}

void MDKPlayer::rotateImage(const qreal value)
{
    if (isStopped()) {
        return;
    }
    if ((value <= 0.0) || (value >= 360.0)) {
        qCWarning(lcQMPMDK) << "The rotation degree should be in the (0, 360) range.";
        return;
    }
    m_player->rotate(static_cast<int>(qRound(value)));
}

void MDKPlayer::scaleImage(const qreal value)
{
    if (isStopped()) {
        return;
    }
    if (value <= 0.0) {
        qCWarning(lcQMPMDK) << "The scale factor should be greater than zero.";
        return;
    }
    m_player->scale(value, value);
}

void MDKPlayer::resetInternalData()
{
    m_lastPosition = 0;
    m_activeVideoTrack = 0;
    m_activeAudioTrack = 0;
    m_activeSubtitleTrack = 0;
    Q_EMIT positionChanged();
    Q_EMIT durationChanged();
    Q_EMIT seekableChanged();
    Q_EMIT chaptersChanged();
    Q_EMIT metaDataChanged();
    Q_EMIT mediaTracksChanged();
    Q_EMIT activeVideoTrackChanged();
    Q_EMIT activeAudioTrackChanged();
    Q_EMIT activeSubtitleTrackChanged();
}

QTMEDIAPLAYER_END_NAMESPACE
