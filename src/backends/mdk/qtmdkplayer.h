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

#pragma once

#include "qtmediaplayer_global.h"
#include "qtmediaplayer.h"
#include <QtCore/qurl.h>
#include <QtCore/qtimer.h>
#include <QtQuick/qquickitem.h>

namespace mdk
{

class Player;

}

QTMEDIAPLAYER_BEGIN_NAMESPACE

class MDKVideoTextureNode;

class QtMDKPlayer : public QtMediaPlayer
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QtMDKPlayer)

    friend class MDKVideoTextureNode;

public:
    explicit QtMDKPlayer(QQuickItem *parent = nullptr);
    ~QtMDKPlayer() override;

    bool backendAvailable() const override;

    QString backendName() const override;
    QString backendVersion() const override;
    QString backendDescription() const override;
    QString backendVendor() const override;
    QString backendCopyright() const override;
    QUrl backendWebsite() const override;

    QUrl source() const override;
    void setSource(const QUrl &value) override;

    QString fileName() const override;

    QString filePath() const override;

    qint64 position() const override;
    void setPosition(const qint64 value) override;

    qint64 duration() const override;

    QSizeF videoSize() const override;

    qreal volume() const override;
    void setVolume(const qreal value) override;

    bool mute() const override;
    void setMute(const bool value) override;

    bool seekable() const override;

    PlaybackState playbackState() const override;
    void setPlaybackState(const PlaybackState value) override;

    MediaStatus mediaStatus() const override;

    LogLevel logLevel() const override;
    void setLogLevel(const LogLevel value) override;

    qreal playbackRate() const override;
    void setPlaybackRate(const qreal value) override;

    qreal aspectRatio() const override;
    void setAspectRatio(const qreal value) override;

    QUrl snapshotDirectory() const override;
    void setSnapshotDirectory(const QUrl &value) override;

    QString snapshotFormat() const override;
    void setSnapshotFormat(const QString &value) override;

    QString snapshotTemplate() const override;
    void setSnapshotTemplate(const QString &value) override;

    bool hardwareDecoding() const override;
    void setHardwareDecoding(const bool value) override;

    bool autoStart() const override;
    void setAutoStart(const bool value) override;

    bool livePreview() const override;
    void setLivePreview(const bool value) override;

    FillMode fillMode() const override;
    void setFillMode(const FillMode value) override;

    Chapters chapters() const override;

    MetaData metaData() const override;

    MediaTracks mediaTracks() const override;

public Q_SLOTS:
    void play() override;
    void pause() override;
    void stop() override;

    void seek(const qint64 value) override;

    void snapshot() override;

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#else
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#endif

private Q_SLOTS:
    void invalidateSceneGraph();

private:
    bool isLoaded() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    void releaseResources() override;

    void initMdkHandlers();

    void resetInternalData();

private:
    MDKVideoTextureNode *m_node = nullptr;

    QTimer m_timer;

    QSharedPointer<mdk::Player> m_player;

    qreal m_volume = 1.0;

    bool m_mute = false;
    bool m_hasVideo = false;
    bool m_hasAudio = false;
    bool m_hasSubtitle = false;
    bool m_hasChapters = false;
    bool m_hardwareDecoding = false;
    bool m_autoStart = true;
    bool m_livePreview = false;

    QUrl m_snapshotDirectory = {};
    QString m_snapshotFormat = QStringLiteral("png");
    QString m_snapshotTemplate = QStringLiteral("${filename}_${datetime}_${frametime}");

    FillMode m_fillMode = FillMode::PreserveAspectFit;
    int m_mediaStatus = 0;
};

QTMEDIAPLAYER_END_NAMESPACE
