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
#include <QtQuick/qquickitem.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

static const QString hardwareDecodingWarningText =
    QStringLiteral("ATTENTION! You are trying to enable hardware decoding. "
                   "While enabling hardware decoding MAY reduce resource consumption, "
                   "it's also a source of error. Software decoding should always be your "
                   "first choice in any case.");

class MediaPlayer : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MediaPlayer)
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(MediaPlayer)
#endif

    Q_PROPERTY(QString backendName READ backendName CONSTANT)
    Q_PROPERTY(QString backendVersion READ backendVersion CONSTANT)
    Q_PROPERTY(QString backendAuthors READ backendAuthors CONSTANT)
    Q_PROPERTY(QString backendCopyright READ backendCopyright CONSTANT)
    Q_PROPERTY(QString backendLicenses READ backendLicenses CONSTANT)
    Q_PROPERTY(QString backendHomepage READ backendHomepage CONSTANT)
    Q_PROPERTY(QString ffmpegVersion READ ffmpegVersion CONSTANT)
    Q_PROPERTY(QString ffmpegConfiguration READ ffmpegConfiguration CONSTANT)
    Q_PROPERTY(QString qtRHIBackendName READ qtRHIBackendName CONSTANT)

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(QSizeF videoSize READ videoSize NOTIFY videoSizeChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool mute READ mute WRITE setMute NOTIFY muteChanged)
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged)
    Q_PROPERTY(PlaybackState playbackState READ playbackState WRITE setPlaybackState NOTIFY playbackStateChanged)
    Q_PROPERTY(MediaStatus mediaStatus READ mediaStatus NOTIFY mediaStatusChanged)
    Q_PROPERTY(LogLevel logLevel READ logLevel WRITE setLogLevel NOTIFY logLevelChanged)
    Q_PROPERTY(qreal playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)
    Q_PROPERTY(qreal aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged)
    Q_PROPERTY(QUrl snapshotDirectory READ snapshotDirectory WRITE setSnapshotDirectory NOTIFY snapshotDirectoryChanged)
    Q_PROPERTY(QString snapshotFormat READ snapshotFormat WRITE setSnapshotFormat NOTIFY snapshotFormatChanged)
    Q_PROPERTY(QString snapshotTemplate READ snapshotTemplate WRITE setSnapshotTemplate NOTIFY snapshotTemplateChanged)
    Q_PROPERTY(QStringList videoFileSuffixes READ videoFileSuffixes CONSTANT)
    Q_PROPERTY(QStringList audioFileSuffixes READ audioFileSuffixes CONSTANT)
    Q_PROPERTY(QStringList subtitleFileSuffixes READ subtitleFileSuffixes CONSTANT)
    Q_PROPERTY(QStringList videoFileMimeTypes READ videoFileMimeTypes CONSTANT)
    Q_PROPERTY(QStringList audioFileMimeTypes READ audioFileMimeTypes CONSTANT)
    Q_PROPERTY(bool hardwareDecoding READ hardwareDecoding WRITE setHardwareDecoding NOTIFY hardwareDecodingChanged)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
    Q_PROPERTY(bool livePreview READ livePreview WRITE setLivePreview NOTIFY livePreviewChanged)
    Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    Q_PROPERTY(Chapters chapters READ chapters NOTIFY chaptersChanged)
    Q_PROPERTY(MetaData metaData READ metaData NOTIFY metaDataChanged)
    Q_PROPERTY(MediaTracks mediaTracks READ mediaTracks NOTIFY mediaTracksChanged)
    Q_PROPERTY(int activeVideoTrack READ activeVideoTrack WRITE setActiveVideoTrack NOTIFY activeVideoTrackChanged)
    Q_PROPERTY(int activeAudioTrack READ activeAudioTrack WRITE setActiveAudioTrack NOTIFY activeAudioTrackChanged)
    Q_PROPERTY(int activeSubtitleTrack READ activeSubtitleTrack WRITE setActiveSubtitleTrack NOTIFY activeSubtitleTrackChanged)
    Q_PROPERTY(QSizeF recommendedWindowSize READ recommendedWindowSize NOTIFY recommendedWindowSizeChanged)
    Q_PROPERTY(QPointF recommendedWindowPosition READ recommendedWindowPosition NOTIFY recommendedWindowPositionChanged)
    Q_PROPERTY(bool rendererReady READ rendererReady NOTIFY rendererReadyChanged)

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
    Q_NODISCARD QString qtRHIBackendName() const;

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

    Q_NODISCARD static inline QStringList videoFileSuffixes()
    {
        static const QStringList list =
        {
            QStringLiteral("*.3g2"),   QStringLiteral("*.3ga"),
            QStringLiteral("*.3gp"),   QStringLiteral("*.3gp2"),
            QStringLiteral("*.3gpp"),  QStringLiteral("*.amv"),
            QStringLiteral("*.asf"),   QStringLiteral("*.asx"),
            QStringLiteral("*.avf"),   QStringLiteral("*.avi"),
            QStringLiteral("*.bdm"),   QStringLiteral("*.bdmv"),
            QStringLiteral("*.bik"),   QStringLiteral("*.clpi"),
            QStringLiteral("*.cpi"),   QStringLiteral("*.dat"),
            QStringLiteral("*.divx"),  QStringLiteral("*.drc"),
            QStringLiteral("*.dv"),    QStringLiteral("*.dvr-ms"),
            QStringLiteral("*.f4v"),   QStringLiteral("*.flv"),
            QStringLiteral("*.gvi"),   QStringLiteral("*.gxf"),
            QStringLiteral("*.hdmov"), QStringLiteral("*.hlv"),
            QStringLiteral("*.iso"),   QStringLiteral("*.letv"),
            QStringLiteral("*.lrv"),   QStringLiteral("*.m1v"),
            QStringLiteral("*.m2p"),   QStringLiteral("*.m2t"),
            QStringLiteral("*.m2ts"),  QStringLiteral("*.m2v"),
            QStringLiteral("*.m3u"),   QStringLiteral("*.m3u8"),
            QStringLiteral("*.m4v"),   QStringLiteral("*.mkv"),
            QStringLiteral("*.moov"),  QStringLiteral("*.mov"),
            QStringLiteral("*.mp2"),   QStringLiteral("*.mp2v"),
            QStringLiteral("*.mp4"),   QStringLiteral("*.mp4v"),
            QStringLiteral("*.mpe"),   QStringLiteral("*.mpeg"),
            QStringLiteral("*.mpeg1"), QStringLiteral("*.mpeg2"),
            QStringLiteral("*.mpeg4"), QStringLiteral("*.mpg"),
            QStringLiteral("*.mpl"),   QStringLiteral("*.mpls"),
            QStringLiteral("*.mpv"),   QStringLiteral("*.mpv2"),
            QStringLiteral("*.mqv"),   QStringLiteral("*.mts"),
            QStringLiteral("*.mtv"),   QStringLiteral("*.mxf"),
            QStringLiteral("*.mxg"),   QStringLiteral("*.nsv"),
            QStringLiteral("*.nuv"),   QStringLiteral("*.ogm"),
            QStringLiteral("*.ogv"),   QStringLiteral("*.ogx"),
            QStringLiteral("*.ps"),    QStringLiteral("*.qt"),
            QStringLiteral("*.qtvr"),  QStringLiteral("*.ram"),
            QStringLiteral("*.rec"),   QStringLiteral("*.rm"),
            QStringLiteral("*.rmj"),   QStringLiteral("*.rmm"),
            QStringLiteral("*.rms"),   QStringLiteral("*.rmvb"),
            QStringLiteral("*.rmx"),   QStringLiteral("*.rp"),
            QStringLiteral("*.rpl"),   QStringLiteral("*.rv"),
            QStringLiteral("*.rvx"),   QStringLiteral("*.thp"),
            QStringLiteral("*.tod"),   QStringLiteral("*.tp"),
            QStringLiteral("*.trp"),   QStringLiteral("*.ts"),
            QStringLiteral("*.tts"),   QStringLiteral("*.txd"),
            QStringLiteral("*.vcd"),   QStringLiteral("*.vdr"),
            QStringLiteral("*.vob"),   QStringLiteral("*.vp8"),
            QStringLiteral("*.vro"),   QStringLiteral("*.webm"),
            QStringLiteral("*.wm"),    QStringLiteral("*.wmv"),
            QStringLiteral("*.wtv"),   QStringLiteral("*.xesc"),
            QStringLiteral("*.xspf")
        };
        return list;
    }

    Q_NODISCARD static inline QStringList audioFileSuffixes()
    {
        static const QStringList list =
        {
            QStringLiteral("*.mp3"),
            QStringLiteral("*.aac"),
            QStringLiteral("*.mka"),
            QStringLiteral("*.dts"),
            QStringLiteral("*.flac"),
            QStringLiteral("*.ogg"),
            QStringLiteral("*.m4a"),
            QStringLiteral("*.ac3"),
            QStringLiteral("*.opus"),
            QStringLiteral("*.wav"),
            QStringLiteral("*.wv")
        };
        return list;
    }

    Q_NODISCARD static inline QStringList subtitleFileSuffixes()
    {
        static const QStringList list =
        {
            QStringLiteral("*.utf"),
            QStringLiteral("*.utf8"),
            QStringLiteral("*.utf-8"),
            QStringLiteral("*.idx"),
            QStringLiteral("*.sub"),
            QStringLiteral("*.srt"),
            QStringLiteral("*.rt"),
            QStringLiteral("*.ssa"),
            QStringLiteral("*.ass"),
            QStringLiteral("*.mks"),
            QStringLiteral("*.vtt"),
            QStringLiteral("*.sup"),
            QStringLiteral("*.scc"),
            QStringLiteral("*.smi")
        };
        return list;
    }

    Q_NODISCARD static QStringList videoFileMimeTypes();
    Q_NODISCARD static QStringList audioFileMimeTypes();

    Q_NODISCARD QSizeF recommendedWindowSize() const;
    Q_NODISCARD QPointF recommendedWindowPosition() const;

    Q_NODISCARD virtual bool rendererReady() const = 0;

public Q_SLOTS:
    virtual void play() = 0;
    void play(const QUrl &url);
    void open(const QUrl &url);
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(const qint64 value) = 0;
    virtual void snapshot() = 0;

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
};

QTMEDIAPLAYER_END_NAMESPACE
