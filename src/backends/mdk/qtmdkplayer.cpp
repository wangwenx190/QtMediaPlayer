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

#include "qtmdkplayer.h"
#include "qtmdkvideotexturenode.h"
#include "qtmdkhelper.h"
#include "include/mdk/Player.h"
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qdatetime.h>
#include <QtQuick/qquickwindow.h>

static inline std::vector<std::string> qStringListToStdStringVector(const QStringList &stringList)
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

static inline QString urlToString(const QUrl &value, const bool display = false)
{
    if (!value.isValid()) {
        return {};
    }
    return (value.isLocalFile() ? QDir::toNativeSeparators(value.toLocalFile())
                                : (display ? value.toDisplayString() : value.url()));
}

static inline MDK_NS_PREPEND(LogLevel) _MDKPlayer_MDK_LogLevel()
{
    return static_cast<MDK_NS_PREPEND(LogLevel)>(MDK_logLevel());
}

QTMEDIAPLAYER_BEGIN_NAMESPACE

MDKVideoTextureNode *createNode(QtMDKPlayer *item);

QtMDKPlayer::QtMDKPlayer(QQuickItem *parent) : QtMediaPlayer(parent)
{
    qDebug() << "Initializing the MDK backend ...";

    m_player.reset(new MDK_NS_PREPEND(Player));
    if (m_player.isNull()) {
        qFatal("Failed to create mdk player.");
    }

    if (!m_livePreview) {
        qDebug() << "Player created.";
    }

    m_player->setRenderCallback([this](void *){
        QMetaObject::invokeMethod(this, "update");
    });

    m_snapshotDirectory = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    connect(this, &QtMDKPlayer::sourceChanged, this, &QtMDKPlayer::fileNameChanged);
    connect(this, &QtMDKPlayer::sourceChanged, this, &QtMDKPlayer::filePathChanged);

    initMdkHandlers();

    m_timer.setTimerType(Qt::CoarseTimer);
    m_timer.setInterval(500);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    m_timer.callOnTimeout(this, [this](){
        if (isPlaying()) {
            Q_EMIT positionChanged();
        }
    });
#else
    connect(&m_timer, &QTimer::timeout, this, [this](){
        if (isPlaying()) {
            Q_EMIT positionChanged();
        }
    });
#endif
    m_timer.start();
}

QtMDKPlayer::~QtMDKPlayer()
{
    if (!m_livePreview) {
        qDebug() << "Player destroyed.";
    }
}

bool QtMDKPlayer::backendAvailable() const
{
    return MDK::Qt::mdkAvailable();
}

QString QtMDKPlayer::backendName() const
{
    return QStringLiteral("MDK");
}

QString QtMDKPlayer::backendVersion() const
{
    // Is this correct?
    return QString::number(MDK_NS_PREPEND(version)());
}

QString QtMDKPlayer::backendDescription() const
{
    // TODO
    return tr("mdk backend.");
}

QString QtMDKPlayer::backendVendor() const
{
    return QStringLiteral("Wang Bin");
}

QString QtMDKPlayer::backendCopyright() const
{
    // TODO
    return {};
}

QUrl QtMDKPlayer::backendHomePage() const
{
    return QStringLiteral("https://sourceforge.net/projects/mdk-sdk/");
}

QString QtMDKPlayer::ffmpegVersion() const
{
    // TODO
    return {};
}

QString QtMDKPlayer::ffmpegConfiguration() const
{
    // TODO
    return {};
}

// The beauty of using a true QSGNode: no need for complicated cleanup
// arrangements, unlike in other examples like metalunderqml, because the
// scenegraph will handle destroying the node at the appropriate time.
void QtMDKPlayer::invalidateSceneGraph() // Called on the render thread when the scenegraph is invalidated.
{
    m_node = nullptr;
}

void QtMDKPlayer::releaseResources() // Called on the gui thread if the item is removed from scene.
{
    m_node = nullptr;
}

QSGNode *QtMDKPlayer::updatePaintNode(QSGNode *node, UpdatePaintNodeData *data)
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
void QtMDKPlayer::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
#else
void QtMDKPlayer::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
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

QUrl QtMDKPlayer::source() const
{
    // ### TODO: isStopped() ?
    if (!m_player->url()) {
        return {};
    }
    return QUrl::fromUserInput(QString::fromUtf8(m_player->url()),
                               QCoreApplication::applicationDirPath(),
                               QUrl::AssumeLocalFile);
}

void QtMDKPlayer::setSource(const QUrl &value)
{
    const auto realStop = [this]() -> void {
        m_player->setNextMedia(nullptr);
        m_player->setState(MDK_NS_PREPEND(PlaybackState)::Stopped);
        m_player->waitFor(MDK_NS_PREPEND(PlaybackState)::Stopped);
    };
    if (value.isEmpty()) {
        qDebug() << "Empty source is set, playback stopped.";
        realStop();
        return;
    }
    if (!value.isValid()) {
        qWarning() << "The given URL" << value << "is invalid.";
        return;
    }
    if (value == source()) {
        if (isStopped() && !m_livePreview) {
            m_player->setState(MDK_NS_PREPEND(PlaybackState)::Playing);
        }
        return;
    }
    realStop();
    // The first url may be the same as current url.
    m_player->setMedia(nullptr);
    m_player->setMedia(qUtf8Printable(urlToString(value)));
    Q_EMIT sourceChanged();
    // It's necessary to call "prepare()" otherwise we'll get no picture.
    m_player->prepare();
    if (m_autoStart && !m_livePreview) {
        m_player->setState(MDK_NS_PREPEND(PlaybackState)::Playing);
    }
}

QString QtMDKPlayer::fileName() const
{
    const QUrl url = source();
    return url.isValid() ? (url.isLocalFile() ? url.fileName() : url.toDisplayString())
                            : QString{};
}

QString QtMDKPlayer::filePath() const
{
    const QUrl url = source();
    return url.isValid() ? urlToString(url, true) : QString{};
}

qint64 QtMDKPlayer::position() const
{
    return isStopped() ? 0 : m_player->position();
}

void QtMDKPlayer::setPosition(const qint64 value)
{
    if (isStopped() || (value == position())) {
        return;
    }
    seek(value);
}

qint64 QtMDKPlayer::duration() const
{
    const auto &mi = m_player->mediaInfo();
    return mi.duration;
}

QSizeF QtMDKPlayer::videoSize() const
{
    const auto &mi = m_player->mediaInfo();
    const auto &vs = mi.video;
    if (vs.empty()) {
        return {};
    }
    const auto &vsf = vs.at(0);
    return {static_cast<qreal>(vsf.codec.width), static_cast<qreal>(vsf.codec.height)};
}

qreal QtMDKPlayer::volume() const
{
    return m_volume;
}

void QtMDKPlayer::setVolume(const qreal value)
{
    if (qFuzzyCompare(value, m_volume)) {
        return;
    }
    if (value < 0.0) {
        qWarning() << "The minimum volume is 0, however" << value << "is given.";
        return;
    }
    if (value > 1.0) {
        qWarning() << "The maximum volume is 1.0, setting a higher number may cause damaged sound.";
    }
    m_volume = value;
    m_player->setVolume(m_volume);
    Q_EMIT volumeChanged();
    if (!m_livePreview) {
        qDebug() << "Volume -->" << m_volume;
    }
}

bool QtMDKPlayer::mute() const
{
    return m_mute;
}

void QtMDKPlayer::setMute(const bool value)
{
    if (value == m_mute) {
        return;
    }
    m_mute = value;
    m_player->setMute(m_mute);
    Q_EMIT muteChanged();
    if (!m_livePreview) {
        qDebug() << "Mute -->" << m_mute;
    }
}

bool QtMDKPlayer::seekable() const
{
    // Local files are always seekable, in theory.
    return (isLoaded() && source().isLocalFile());
}

QtMDKPlayer::PlaybackState QtMDKPlayer::playbackState() const
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

void QtMDKPlayer::setPlaybackState(const QtMDKPlayer::PlaybackState value)
{
    if (value == playbackState()) {
        return;
    }
    switch (value) {
    case PlaybackState::Playing: {
        if (!m_livePreview) {
            m_player->setState(MDK_NS_PREPEND(PlaybackState)::Playing);
        }
    } break;
    case PlaybackState::Paused:
        m_player->setState(MDK_NS_PREPEND(PlaybackState)::Paused);
        break;
    case PlaybackState::Stopped:
        m_player->setState(MDK_NS_PREPEND(PlaybackState)::Stopped);
        break;
    }
}

QtMDKPlayer::MediaStatus QtMDKPlayer::mediaStatus() const
{
    const auto ms = m_player->mediaStatus();
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::NoMedia)) {
        return MediaStatus::NoMedia;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::Unloaded)) {
        return MediaStatus::Unloaded;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::Loading)) {
        return MediaStatus::Loading;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::Loaded)) {
        return MediaStatus::Loaded;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::Prepared)) {
        return MediaStatus::Prepared;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::Stalled)) {
        return MediaStatus::Stalled;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::Buffering)) {
        return MediaStatus::Buffering;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::Buffered)) {
        return MediaStatus::Buffered;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::End)) {
        return MediaStatus::End;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::Seeking)) {
        return MediaStatus::Seeking;
    }
    if (MDK_NS_PREPEND(test_flag)(ms & MDK_NS_PREPEND(MediaStatus)::Invalid)) {
        return MediaStatus::Invalid;
    }
    return MediaStatus::Invalid;
}

QtMDKPlayer::LogLevel QtMDKPlayer::logLevel() const
{
    switch (_MDKPlayer_MDK_LogLevel()) {
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

void QtMDKPlayer::setLogLevel(const QtMDKPlayer::LogLevel value)
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
        qDebug() << "Log level -->" << value;
    }
}

qreal QtMDKPlayer::playbackRate() const
{
    return static_cast<qreal>(m_player->playbackRate());
}

void QtMDKPlayer::setPlaybackRate(const qreal value)
{
    if (qFuzzyCompare(value, playbackRate())) {
        return;
    }
    m_player->setPlaybackRate(value);
    Q_EMIT playbackRateChanged();
    if (!m_livePreview) {
        qDebug() << "Playback rate -->" << value;
    }
}

qreal QtMDKPlayer::aspectRatio() const
{
    const QSizeF vs = videoSize();
    return (vs.width() / vs.height());
}

void QtMDKPlayer::setAspectRatio(const qreal value)
{
    if (qFuzzyCompare(value, aspectRatio())) {
        return;
    }
    m_player->setAspectRatio(value);
    Q_EMIT aspectRatioChanged();
    if (!m_livePreview) {
        qDebug() << "Aspect ratio -->" << value;
    }
}

QUrl QtMDKPlayer::snapshotDirectory() const
{
    return m_snapshotDirectory;
}

void QtMDKPlayer::setSnapshotDirectory(const QUrl &value)
{
    if (value.isEmpty() || (value == snapshotDirectory())) {
        return;
    }
    m_snapshotDirectory = value;
    Q_EMIT snapshotDirectoryChanged();
    if (!m_livePreview) {
        qDebug() << "Snapshot directory -->" << m_snapshotDirectory;
    }
}

QString QtMDKPlayer::snapshotFormat() const
{
    return m_snapshotFormat;
}

void QtMDKPlayer::setSnapshotFormat(const QString &value)
{
    if (value.isEmpty() || (value == m_snapshotFormat)) {
        return;
    }
    m_snapshotFormat = value.toLower();
    Q_EMIT snapshotFormatChanged();
    if (!m_livePreview) {
        qDebug() << "Snapshot format -->" << m_snapshotFormat;
    }
}

QString QtMDKPlayer::snapshotTemplate() const
{
    return m_snapshotTemplate;
}

void QtMDKPlayer::setSnapshotTemplate(const QString &value)
{
    if (value.isEmpty() || (value == m_snapshotTemplate)) {
        return;
    }
    m_snapshotTemplate = value;
    Q_EMIT snapshotTemplateChanged();
    if (!m_livePreview) {
        qDebug() << "Snapshot template -->" << m_snapshotTemplate;
    }
}

bool QtMDKPlayer::hardwareDecoding() const
{
    return m_hardwareDecoding;
}

void QtMDKPlayer::setHardwareDecoding(const bool value)
{
    const auto setVideoDecoders = [this](const QStringList &decoders) -> void
    {
        const QStringList videoDecoders = decoders.isEmpty() ? QStringList{QStringLiteral("FFmpeg")} : decoders;
        m_player->setDecoders(MDK_NS_PREPEND(MediaType)::Video, qStringListToStdStringVector(videoDecoders));
        if (!m_livePreview) {
            qDebug() << "Video decoders -->" << videoDecoders;
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
#elif defined(Q_OS_LINUX)
        QStringLiteral("VAAPI"),
        QStringLiteral("VDPAU"),
        QStringLiteral("CUDA"),
        QStringLiteral("NVDEC"),
#elif defined(Q_OS_DARWIN)
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
            setVideoDecoders(defaultVideoDecoders);
        } else {
            setVideoDecoders({QStringLiteral("FFmpeg")});
        }
        Q_EMIT hardwareDecodingChanged();
        if (!m_livePreview) {
            qDebug() << "Hardware decoding -->" << m_hardwareDecoding;
        }
    }
}

bool QtMDKPlayer::autoStart() const
{
    return m_autoStart;
}

void QtMDKPlayer::setAutoStart(const bool value)
{
    if (m_autoStart != value) {
        m_autoStart = value;
        Q_EMIT autoStartChanged();
        if (!m_livePreview) {
            qDebug() << "Auto start -->" << m_autoStart;
        }
    }
}

bool QtMDKPlayer::livePreview() const
{
    return m_livePreview;
}

void QtMDKPlayer::setLivePreview(const bool value)
{
    if (m_livePreview != value) {
        m_livePreview = value;
        if (m_livePreview) {
            // We only need static images.
            m_player->setState(MDK_NS_PREPEND(PlaybackState)::Paused);
            // We don't want the preview window play sound.
            m_player->setMute(true);
            // Decode as soon as possible when media data received.
            m_player->setBufferRange(0);
            // Prevent player stop playing after EOF is reached.
            m_player->setProperty("continue_at_end", "1");
            // And don't forget to use accurate seek.
        } else {
            // Restore everything to default.
            m_player->setBufferRange(1000);
            m_player->setMute(m_mute);
            m_player->setProperty("continue_at_end", "0");
        }
        Q_EMIT livePreviewChanged();
    }
}

QtMDKPlayer::FillMode QtMDKPlayer::fillMode() const
{
    return m_fillMode;
}

void QtMDKPlayer::setFillMode(const QtMDKPlayer::FillMode value)
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
            qDebug() << "Fill mode -->" << m_fillMode;
        }
    }
}

QtMediaPlayer::Chapters QtMDKPlayer::chapters() const
{
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
        result.append(info);
    }
    return result;
}

QtMediaPlayer::MetaData QtMDKPlayer::metaData() const
{
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

QtMediaPlayer::MediaTracks QtMDKPlayer::mediaTracks() const
{
    const auto &mi = m_player->mediaInfo();
    const auto &vs = mi.video;
    const auto &as = mi.audio;
    MediaTracks result = {};
    if (!vs.empty()) {
        for (auto &&vsi : qAsConst(vs)) {
            QVariantHash info = {};
            info.insert(QStringLiteral("index"), vsi.index);
            info.insert(QStringLiteral("start_time"), vsi.start_time);
            info.insert(QStringLiteral("duration"), vsi.duration);
            info.insert(QStringLiteral("width"), vsi.codec.width);
            info.insert(QStringLiteral("height"), vsi.codec.height);
            info.insert(QStringLiteral("frame_rate"), vsi.codec.frame_rate);
            info.insert(QStringLiteral("bit_rate"), vsi.codec.bit_rate);
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
            info.insert(QStringLiteral("start_time"), asi.start_time);
            info.insert(QStringLiteral("duration"), asi.duration);
            info.insert(QStringLiteral("bit_rate"), asi.codec.bit_rate);
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

void QtMDKPlayer::play()
{
    if (!source().isValid() || m_livePreview) {
        return;
    }
    m_player->setState(MDK_NS_PREPEND(PlaybackState)::Playing);
}

void QtMDKPlayer::pause()
{
    if (!source().isValid()) {
        return;
    }
    m_player->setState(MDK_NS_PREPEND(PlaybackState)::Paused);
}

void QtMDKPlayer::stop()
{
    if (!source().isValid()) {
        return;
    }
    m_player->setNextMedia(nullptr);
    m_player->setState(MDK_NS_PREPEND(PlaybackState)::Stopped);
    m_player->waitFor(MDK_NS_PREPEND(PlaybackState)::Stopped);
}

void QtMDKPlayer::seek(const qint64 value)
{
    if (isStopped() || (value == position())) {
        return;
    }
    // We have to seek accurately when we are in live preview mode.
    m_player->seek(value, m_livePreview ? MDK_NS_PREPEND(SeekFlag)::FromStart : MDK_NS_PREPEND(SeekFlag)::Default);
    // In case the playback is paused.
    Q_EMIT positionChanged();
    if (!m_livePreview) {
        qDebug() << "Seek -->" << value;
    }
}

void QtMDKPlayer::snapshot()
{
    if (isStopped()) {
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
            qDebug() << "Taking snapshot -->" << path;
        }
        return qstrdup(qUtf8Printable(path));
    });
}

void QtMDKPlayer::initMdkHandlers()
{
    MDK_NS_PREPEND(setLogHandler)([this](MDK_NS_PREPEND(LogLevel) level, const char *msg) {
        QString prefix = objectName();
        if (m_livePreview) {
            if (prefix.isEmpty()) {
                prefix = QStringLiteral("<TIME LINE PREVIEW>");
            } else {
                prefix.append(QStringLiteral(" (time line preview)"));
            }
        }
        switch (level) {
        case MDK_NS_PREPEND(LogLevel)::Info:
            qInfo().noquote() << prefix << msg;
            break;
        case MDK_NS_PREPEND(LogLevel)::All:
        case MDK_NS_PREPEND(LogLevel)::Debug:
            qDebug().noquote() << prefix << msg;
            break;
        case MDK_NS_PREPEND(LogLevel)::Warning:
            qWarning().noquote() << prefix << msg;
            break;
        case MDK_NS_PREPEND(LogLevel)::Error:
            qCritical().noquote() << prefix << msg;
            break;
        default:
            break;
        }
    });
    m_player->currentMediaChanged([this] {
        const QUrl url = source();
        if (!url.isValid()) {
            return;
        }
        if (!m_livePreview) {
            qDebug() << "Current media -->" << urlToString(url, true);
        }
        Q_EMIT sourceChanged();
    });
    m_player->onMediaStatusChanged([this](MDK_NS_PREPEND(MediaStatus) ms) {
        if (MDK_NS_PREPEND(flags_added)(static_cast<MDK_NS_PREPEND(MediaStatus)>(m_mediaStatus), ms, MDK_NS_PREPEND(MediaStatus)::Loaded)) {
            Q_EMIT videoSizeChanged();
            Q_EMIT positionChanged();
            Q_EMIT durationChanged();
            Q_EMIT seekableChanged();
            Q_EMIT chaptersChanged();
            Q_EMIT metaDataChanged();
            Q_EMIT mediaTracksChanged();
            Q_EMIT loaded();
            if (!m_livePreview) {
                qDebug() << "Media loaded.";
            }
        }
        m_mediaStatus = static_cast<int>(ms);
        Q_EMIT mediaStatusChanged();
        return true;
    });
    m_player->onEvent([this](const MDK_NS_PREPEND(MediaEvent) &me) {
        if (!m_livePreview) {
            qDebug() << "MDK event:" << me.category.data() << me.detail.data();
        }
        return false;
    });
    m_player->onLoop([this](int count) {
        if (!m_livePreview) {
            qDebug() << "loop:" << count;
        }
        return false;
    });
    m_player->onStateChanged([this](MDK_NS_PREPEND(PlaybackState) pbs) {
        Q_EMIT playbackStateChanged();
        if (pbs == MDK_NS_PREPEND(PlaybackState)::Playing) {
            Q_EMIT playing();
            if (!m_livePreview) {
                qDebug() << "Start playing.";
            }
        }
        if (pbs == MDK_NS_PREPEND(PlaybackState)::Paused) {
            Q_EMIT paused();
            if (!m_livePreview) {
                qDebug() << "Paused.";
            }
        }
        if (pbs == MDK_NS_PREPEND(PlaybackState)::Stopped) {
            resetInternalData();
            Q_EMIT stopped();
            if (!m_livePreview) {
                qDebug() << "Stopped.";
            }
        }
    });
}

void QtMDKPlayer::resetInternalData()
{
    // Make sure MDKPlayer::url() returns empty.
    m_player->setMedia(nullptr);
    m_mediaStatus = static_cast<int>(MDK_NS_PREPEND(MediaStatus)::NoMedia);
    Q_EMIT sourceChanged();
    Q_EMIT positionChanged();
    Q_EMIT durationChanged();
    Q_EMIT seekableChanged();
    Q_EMIT chaptersChanged();
    Q_EMIT metaDataChanged();
    Q_EMIT mediaTracksChanged();
    Q_EMIT mediaStatusChanged();
}

bool QtMDKPlayer::isLoaded() const
{
    const MediaStatus ms = mediaStatus();
    return ((ms != MediaStatus::Invalid)
            && (ms != MediaStatus::NoMedia)
            && (ms != MediaStatus::Unloaded)
            && (ms != MediaStatus::Loading));
}

bool QtMDKPlayer::isPlaying() const
{
    return (m_player->state() == MDK_NS_PREPEND(PlaybackState)::Playing);
}

bool QtMDKPlayer::isPaused() const
{
    return (m_player->state() == MDK_NS_PREPEND(PlaybackState)::Paused);
}

bool QtMDKPlayer::isStopped() const
{
    return (m_player->state() == MDK_NS_PREPEND(PlaybackState)::Stopped);
}

QTMEDIAPLAYER_END_NAMESPACE
