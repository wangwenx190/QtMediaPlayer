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

#include "playertypes.h"
#include "mediainfo.h"
#include <QtQuick/qquickitem.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

static const QString hardwareDecodingWarningText =
    QStringLiteral("ATTENTION! You are trying to enable hardware decoding. "
                   "While enabling hardware decoding MAY reduce resource consumption, "
                   "it's also a source of error. Software decoding should always be your "
                   "first choice in any case.");

class QTMEDIAPLAYER_COMMON_API MediaPlayer : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MediaPlayer)

    Q_PROPERTY(QString backendName READ backendName CONSTANT FINAL)
    Q_PROPERTY(QString backendVersion READ backendVersion CONSTANT FINAL)
    Q_PROPERTY(QString backendAuthors READ backendAuthors CONSTANT FINAL)
    Q_PROPERTY(QString backendCopyright READ backendCopyright CONSTANT FINAL)
    Q_PROPERTY(QString backendLicenses READ backendLicenses CONSTANT FINAL)
    Q_PROPERTY(QString backendHomepage READ backendHomepage CONSTANT FINAL)
    Q_PROPERTY(QString ffmpegVersion READ ffmpegVersion CONSTANT FINAL)
    Q_PROPERTY(QString ffmpegConfiguration READ ffmpegConfiguration CONSTANT FINAL)
    Q_PROPERTY(QString graphicsApiName READ graphicsApiName CONSTANT FINAL)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged FINAL)
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged FINAL)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged FINAL)
    Q_PROPERTY(QSizeF videoSize READ videoSize NOTIFY videoSizeChanged FINAL)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)
    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged FINAL)
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged FINAL)
    Q_PROPERTY(PlaybackState playbackState READ playbackState WRITE setPlaybackState NOTIFY playbackStateChanged FINAL)
    Q_PROPERTY(MediaStatus mediaStatus READ mediaStatus NOTIFY mediaStatusChanged FINAL)
    Q_PROPERTY(LogLevel logLevel READ logLevel WRITE setLogLevel NOTIFY logLevelChanged FINAL)
    Q_PROPERTY(qreal playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged FINAL)
    Q_PROPERTY(qreal aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged FINAL)
    Q_PROPERTY(QUrl snapshotDirectory READ snapshotDirectory WRITE setSnapshotDirectory NOTIFY snapshotDirectoryChanged FINAL)
    Q_PROPERTY(QString snapshotFormat READ snapshotFormat WRITE setSnapshotFormat NOTIFY snapshotFormatChanged FINAL)
    Q_PROPERTY(QString snapshotTemplate READ snapshotTemplate WRITE setSnapshotTemplate NOTIFY snapshotTemplateChanged FINAL)
    Q_PROPERTY(QStringList videoFileSuffixes READ videoFileSuffixes CONSTANT FINAL)
    Q_PROPERTY(QStringList audioFileSuffixes READ audioFileSuffixes CONSTANT FINAL)
    Q_PROPERTY(QStringList subtitleFileSuffixes READ subtitleFileSuffixes CONSTANT FINAL)
    Q_PROPERTY(QStringList videoFileMimeTypes READ videoFileMimeTypes CONSTANT FINAL)
    Q_PROPERTY(QStringList audioFileMimeTypes READ audioFileMimeTypes CONSTANT FINAL)
    Q_PROPERTY(bool hardwareDecoding READ hardwareDecoding WRITE setHardwareDecoding NOTIFY hardwareDecodingChanged FINAL)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged FINAL)
    Q_PROPERTY(bool livePreview READ livePreview WRITE setLivePreview NOTIFY livePreviewChanged FINAL)
    Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged FINAL)
    Q_PROPERTY(Chapters chapters READ chapters NOTIFY chaptersChanged FINAL)
    Q_PROPERTY(MetaData metaData READ metaData NOTIFY metaDataChanged FINAL)
    Q_PROPERTY(MediaTracks mediaTracks READ mediaTracks NOTIFY mediaTracksChanged FINAL)
    Q_PROPERTY(int activeVideoTrack READ activeVideoTrack WRITE setActiveVideoTrack NOTIFY activeVideoTrackChanged FINAL)
    Q_PROPERTY(int activeAudioTrack READ activeAudioTrack WRITE setActiveAudioTrack NOTIFY activeAudioTrackChanged FINAL)
    Q_PROPERTY(int activeSubtitleTrack READ activeSubtitleTrack WRITE setActiveSubtitleTrack NOTIFY activeSubtitleTrackChanged FINAL)
    Q_PROPERTY(QSizeF recommendedWindowSize READ recommendedWindowSize NOTIFY recommendedWindowSizeChanged FINAL)
    Q_PROPERTY(QPointF recommendedWindowPosition READ recommendedWindowPosition NOTIFY recommendedWindowPositionChanged FINAL)
    Q_PROPERTY(bool rendererReady READ rendererReady NOTIFY rendererReadyChanged FINAL)
    Q_PROPERTY(bool hasVideo READ hasVideo NOTIFY hasVideoChanged FINAL)
    Q_PROPERTY(bool hasAudio READ hasAudio NOTIFY hasAudioChanged FINAL)
    Q_PROPERTY(bool hasSubtitle READ hasSubtitle NOTIFY hasSubtitleChanged FINAL)
    Q_PROPERTY(MediaInfo* mediaInfo READ mediaInfo CONSTANT FINAL)

public:
    explicit MediaPlayer(QQuickItem *parent = nullptr);
    virtual ~MediaPlayer() override;

    Q_NODISCARD virtual QString backendName() const = 0;
    Q_NODISCARD virtual QString backendVersion() const = 0;
    Q_NODISCARD virtual QString backendAuthors() const = 0;
    Q_NODISCARD virtual QString backendCopyright() const = 0;
    Q_NODISCARD virtual QString backendLicenses() const = 0;
    Q_NODISCARD virtual QString backendHomepage() const = 0;
    Q_NODISCARD virtual QString ffmpegVersion() const = 0;
    Q_NODISCARD virtual QString ffmpegConfiguration() const = 0;
    Q_NODISCARD QString graphicsApiName() const;

    Q_NODISCARD virtual QUrl source() const = 0;
    virtual void setSource(const QUrl &value) = 0;

    Q_NODISCARD virtual QString fileName() const = 0;

    Q_NODISCARD virtual QString filePath() const = 0;

    Q_NODISCARD virtual qint64 position() const = 0;
    virtual void setPosition(const qint64 value) = 0;

    Q_NODISCARD virtual qint64 duration() const = 0;

    Q_NODISCARD virtual QSizeF videoSize() const = 0;

    Q_NODISCARD virtual qreal volume() const = 0;
    virtual void setVolume(const qreal value) = 0;

    Q_NODISCARD virtual bool mute() const = 0;
    virtual void setMute(const bool value) = 0;

    Q_NODISCARD virtual bool seekable() const = 0;

    Q_NODISCARD virtual PlaybackState playbackState() const = 0;
    virtual void setPlaybackState(const PlaybackState value) = 0;

    Q_NODISCARD virtual MediaStatus mediaStatus() const = 0;

    Q_NODISCARD virtual LogLevel logLevel() const = 0;
    virtual void setLogLevel(const LogLevel value) = 0;

    Q_NODISCARD virtual qreal playbackRate() const = 0;
    virtual void setPlaybackRate(const qreal value) = 0;

    Q_NODISCARD virtual qreal aspectRatio() const = 0;
    virtual void setAspectRatio(const qreal value) = 0;

    Q_NODISCARD virtual QUrl snapshotDirectory() const = 0;
    virtual void setSnapshotDirectory(const QUrl &value) = 0;

    Q_NODISCARD virtual QString snapshotFormat() const = 0;
    virtual void setSnapshotFormat(const QString &value) = 0;

    Q_NODISCARD virtual QString snapshotTemplate() const = 0;
    virtual void setSnapshotTemplate(const QString &value) = 0;

    Q_NODISCARD virtual bool hardwareDecoding() const = 0;
    virtual void setHardwareDecoding(const bool value) = 0;

    Q_NODISCARD virtual bool autoStart() const = 0;
    virtual void setAutoStart(const bool value) = 0;

    Q_NODISCARD virtual bool livePreview() const = 0;
    virtual void setLivePreview(const bool value) = 0;

    Q_NODISCARD virtual FillMode fillMode() const = 0;
    virtual void setFillMode(const FillMode value) = 0;

    Q_NODISCARD virtual Chapters chapters() const = 0;

    Q_NODISCARD virtual MetaData metaData() const = 0;

    Q_NODISCARD virtual MediaTracks mediaTracks() const = 0;

    Q_NODISCARD virtual int activeVideoTrack() const = 0;
    virtual void setActiveVideoTrack(const int value) = 0;

    Q_NODISCARD virtual int activeAudioTrack() const = 0;
    virtual void setActiveAudioTrack(const int value) = 0;

    Q_NODISCARD virtual int activeSubtitleTrack() const = 0;
    virtual void setActiveSubtitleTrack(const int value) = 0;

    Q_NODISCARD static QStringList videoFileSuffixes();
    Q_NODISCARD static QStringList audioFileSuffixes();
    Q_NODISCARD static QStringList subtitleFileSuffixes();

    Q_NODISCARD static QStringList videoFileMimeTypes();
    Q_NODISCARD static QStringList audioFileMimeTypes();

    Q_NODISCARD QSizeF recommendedWindowSize() const;
    Q_NODISCARD QPointF recommendedWindowPosition() const;

    Q_NODISCARD virtual bool rendererReady() const = 0;

    Q_NODISCARD bool hasVideo() const;
    Q_NODISCARD bool hasAudio() const;
    Q_NODISCARD bool hasSubtitle() const;

    Q_NODISCARD MediaInfo *mediaInfo() const;

public Q_SLOTS:
    virtual void play() = 0;
    void play(const QUrl &url);
    void open(const QUrl &url);
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(const qint64 value) = 0;
    virtual void snapshot() = 0;
    virtual void rotateImage(const qreal value) = 0;
    virtual void scaleImage(const qreal value) = 0;
    void nextChapter();
    void previousChapter();

public:
    Q_NODISCARD Q_INVOKABLE virtual bool isLoaded() const = 0;
    Q_NODISCARD Q_INVOKABLE virtual bool isPlaying() const = 0;
    Q_NODISCARD Q_INVOKABLE virtual bool isPaused() const = 0;
    Q_NODISCARD Q_INVOKABLE virtual bool isStopped() const = 0;

    Q_NODISCARD Q_INVOKABLE static QString formatTime
        (const qint64 ms, const QString &pattern = QStringLiteral("hh:mm:ss"));

    Q_NODISCARD Q_INVOKABLE static bool isVideoFile(const QString &fileName);
    Q_NODISCARD Q_INVOKABLE static bool isAudioFile(const QString &fileName);
    Q_NODISCARD Q_INVOKABLE static bool isSubtitleFile(const QString &fileName);
    Q_NODISCARD Q_INVOKABLE static bool isMediaFile(const QString &fileName);

    Q_NODISCARD Q_INVOKABLE bool isPlayingVideo() const;
    Q_NODISCARD Q_INVOKABLE bool isPlayingAudio() const;

Q_SIGNALS:
    void loaded();
    void playing();
    void paused();
    void stopped();
    void stoppedWithPosition(const QUrl &url, const qint64 pos);

    void sourceChanged();
    void fileNameChanged();
    void filePathChanged();
    void positionChanged();
    void durationChanged();
    void videoSizeChanged();
    void volumeChanged();
    void muteChanged();
    void seekableChanged();
    void playbackStateChanged();
    void mediaStatusChanged();
    void logLevelChanged();
    void playbackRateChanged();
    void aspectRatioChanged();
    void snapshotDirectoryChanged();
    void snapshotFormatChanged();
    void snapshotTemplateChanged();
    void hardwareDecodingChanged();
    void autoStartChanged();
    void livePreviewChanged();
    void fillModeChanged();
    void chaptersChanged();
    void metaDataChanged();
    void mediaTracksChanged();
    void activeVideoTrackChanged();
    void activeAudioTrackChanged();
    void activeSubtitleTrackChanged();
    void recommendedWindowSizeChanged();
    void recommendedWindowPositionChanged();
    void rendererReadyChanged();
    void hasVideoChanged();
    void hasAudioChanged();
    void hasSubtitleChanged();

private:
    QScopedPointer<MediaInfo> m_mediaInfo;
};

QTMEDIAPLAYER_END_NAMESPACE

Q_DECLARE_METATYPE(QTMEDIAPLAYER_PREPEND_NAMESPACE(MediaPlayer))
QML_DECLARE_TYPE(QTMEDIAPLAYER_PREPEND_NAMESPACE(MediaPlayer))
