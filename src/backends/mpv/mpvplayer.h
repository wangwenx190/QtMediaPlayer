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

#pragma once

#include "mpvbackend_global.h"
#include <playerinterface.h>

struct mpv_handle;
struct mpv_render_context;

QTMEDIAPLAYER_BEGIN_NAMESPACE

class MPVVideoTextureNode;

class MPVPlayer : public MediaPlayer
{
    Q_OBJECT
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(MediaPlayer)
#endif
    Q_DISABLE_COPY_MOVE(MPVPlayer)

    friend class MPVVideoTextureNode;

public:
    explicit MPVPlayer(QQuickItem *parent = nullptr);
    ~MPVPlayer() override;

    static void on_update(void *ctx);

    Q_NODISCARD QString backendName() const override;
    Q_NODISCARD QString backendVersion() const override;
    Q_NODISCARD QString backendAuthors() const override;
    Q_NODISCARD QString backendCopyright() const override;
    Q_NODISCARD QString backendLicenses() const override;
    Q_NODISCARD QString backendHomepage() const override;
    Q_NODISCARD QString ffmpegVersion() const override;
    Q_NODISCARD QString ffmpegConfiguration() const override;

    Q_NODISCARD QUrl source() const override;
    void setSource(const QUrl &value) override;

    Q_NODISCARD QString fileName() const override;

    Q_NODISCARD QString filePath() const override;

    Q_NODISCARD qint64 position() const override;
    void setPosition(const qint64 value) override;

    Q_NODISCARD qint64 duration() const override;

    Q_NODISCARD QSizeF videoSize() const override;

    Q_NODISCARD qreal volume() const override;
    void setVolume(const qreal value) override;

    Q_NODISCARD bool mute() const override;
    void setMute(const bool value) override;

    Q_NODISCARD bool seekable() const override;

    Q_NODISCARD PlaybackState playbackState() const override;
    void setPlaybackState(const PlaybackState value) override;

    Q_NODISCARD MediaStatus mediaStatus() const override;

    Q_NODISCARD LogLevel logLevel() const override;
    void setLogLevel(const LogLevel value) override;

    Q_NODISCARD qreal playbackRate() const override;
    void setPlaybackRate(const qreal value) override;

    Q_NODISCARD qreal aspectRatio() const override;
    void setAspectRatio(const qreal value) override;

    Q_NODISCARD QUrl snapshotDirectory() const override;
    void setSnapshotDirectory(const QUrl &value) override;

    Q_NODISCARD QString snapshotFormat() const override;
    void setSnapshotFormat(const QString &value) override;

    Q_NODISCARD QString snapshotTemplate() const override;
    void setSnapshotTemplate(const QString &value) override;

    Q_NODISCARD bool hardwareDecoding() const override;
    void setHardwareDecoding(const bool value) override;

    Q_NODISCARD bool autoStart() const override;
    void setAutoStart(const bool value) override;

    Q_NODISCARD bool livePreview() const override;
    void setLivePreview(const bool value) override;

    Q_NODISCARD FillMode fillMode() const override;
    void setFillMode(const FillMode value) override;

    Q_NODISCARD Chapters chapters() const override;

    Q_NODISCARD MetaData metaData() const override;

    Q_NODISCARD MediaTracks mediaTracks() const override;

    Q_NODISCARD int activeVideoTrack() const override;
    void setActiveVideoTrack(const int value) override;

    Q_NODISCARD int activeAudioTrack() const override;
    void setActiveAudioTrack(const int value) override;

    Q_NODISCARD int activeSubtitleTrack() const override;
    void setActiveSubtitleTrack(const int value) override;

    Q_NODISCARD bool rendererReady() const override;

public Q_SLOTS:
    void play() override;
    void pause() override;
    void stop() override;
    void seek(const qint64 value) override;
    void snapshot() override;
    void rotateImage(const qreal value) override;
    void scaleImage(const qreal value) override;

public:
    Q_NODISCARD Q_INVOKABLE bool isLoaded() const override;
    Q_NODISCARD Q_INVOKABLE bool isPlaying() const override;
    Q_NODISCARD Q_INVOKABLE bool isPaused() const override;
    Q_NODISCARD Q_INVOKABLE bool isStopped() const override;

protected Q_SLOTS:
    void handleMpvEvents();

protected:
    Q_NODISCARD QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#else
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#endif

private Q_SLOTS:
    void doUpdate();
    void invalidateSceneGraph();
    void setRendererReady(const bool value);

private:
    void initialize();
    void deinitialize();

    void releaseResources() override;

    Q_NODISCARD bool mpvSendCommand(const QVariant &arguments);
    Q_NODISCARD bool mpvSetProperty(const QString &name, const QVariant &value);
    Q_NODISCARD QVariant mpvGetProperty(const QString &name, const bool silent = false, bool *ok = nullptr) const;
    Q_NODISCARD bool mpvObserveProperty(const QString &name);

    void processMpvLogMessage(void *event);
    void processMpvPropertyChange(void *event);

    void videoReconfig();
    void audioReconfig();

Q_SIGNALS:
    void onUpdate();
    void hasMpvEvents();

private:
    mpv_handle *m_mpv = nullptr;
    mpv_render_context *m_mpv_gl = nullptr;

    MPVVideoTextureNode *m_node = nullptr;

    QUrl m_source = {};
    QUrl m_cachedUrl = {};
    MediaStatus m_mediaStatus = {};
    bool m_livePreview = false;
    bool m_autoStart = true;
    qint64 m_lastPosition = 0;
    bool m_rendererReady = false;
    bool m_loaded = false;

    static inline const QHash<QString, QList<const char *>> properties =
    {
        {QStringLiteral("dwidth"), {"videoSizeChanged"}},
        {QStringLiteral("dheight"), {"videoSizeChanged"}},
        {QStringLiteral("duration"), {"durationChanged"}},
        {QStringLiteral("time-pos"), {"positionChanged"}},
        {QStringLiteral("volume"), {"volumeChanged"}},
        {QStringLiteral("mute"), {"muteChanged"}},
        {QStringLiteral("seekable"), {"seekableChanged"}},
        {QStringLiteral("hwdec"), {"hardwareDecodingChanged"}},
        {QStringLiteral("video-out-params/aspect"), {"aspectRatioChanged"}},
        {QStringLiteral("speed"), {"playbackRateChanged"}},
        {QStringLiteral("filename"), {"fileNameChanged"}},
        {QStringLiteral("screenshot-format"), {"snapshotFormatChanged"}},
        {QStringLiteral("screenshot-template"), {"snapshotTemplateChanged"}},
        {QStringLiteral("screenshot-directory"), {"snapshotDirectoryChanged"}},
        {QStringLiteral("path"), {"filePathChanged"}},
        {QStringLiteral("pause"), {"playbackStateChanged"}},
        {QStringLiteral("idle-active"), {"playbackStateChanged"}},
        {QStringLiteral("track-list"), {"mediaTracksChanged"}},
        {QStringLiteral("chapter-list"), {"chaptersChanged"}},
        {QStringLiteral("metadata"), {"metaDataChanged"}},
        {QStringLiteral("video-unscaled"), {"fillModeChanged"}},
        {QStringLiteral("keepaspect"), {"fillModeChanged"}},
        {QStringLiteral("vid"), {"activeVideoTrackChanged"}},
        {QStringLiteral("aid"), {"activeAudioTrackChanged"}},
        {QStringLiteral("sid"), {"activeSubtitleTrackChanged"}}
    };

    // These properties are changing all the time during the playback process.
    // So we have to add them to the black list, otherwise we'll get huge
    // message floods.
    static inline const QStringList propertyBlackList =
    {
        QStringLiteral("time-pos"),
        QStringLiteral("playback-time"),
        QStringLiteral("percent-pos"),
        QStringLiteral("video-bitrate"),
        QStringLiteral("audio-bitrate"),
        QStringLiteral("estimated-vf-fps"),
        QStringLiteral("avsync")
    };
};

QTMEDIAPLAYER_END_NAMESPACE
