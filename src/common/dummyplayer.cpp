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

#include "dummyplayer.h"

QTMEDIAPLAYER_BEGIN_NAMESPACE

DummyPlayer::DummyPlayer(QQuickItem *parent) : MediaPlayer(parent) {}

DummyPlayer::~DummyPlayer() = default;

QString DummyPlayer::backendName() const
{
    return QStringLiteral("Dummy");
}

QString DummyPlayer::backendVersion() const
{
    return QStringLiteral("1.0.0");
}

QString DummyPlayer::backendAuthors() const
{
    return QStringLiteral("wangwenx190");
}

QString DummyPlayer::backendCopyright() const
{
    return QStringLiteral("Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)");
}

QString DummyPlayer::backendLicenses() const
{
    return QStringLiteral(R"(MIT License

Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)");
}

QString DummyPlayer::backendHomepage() const
{
    return QStringLiteral("https://github.com/wangwenx190/QtMediaPlayer/");
}

QString DummyPlayer::ffmpegVersion() const
{
    return QStringLiteral("Unknown");
}

QString DummyPlayer::ffmpegConfiguration() const
{
    return QStringLiteral("Unknown");
}

QUrl DummyPlayer::source() const
{
    return {};
}

void DummyPlayer::setSource(const QUrl &value)
{
    Q_UNUSED(value);
}

QString DummyPlayer::fileName() const
{
    return {};
}

QString DummyPlayer::filePath() const
{
    return {};
}

qint64 DummyPlayer::position() const
{
    return 0;
}

void DummyPlayer::setPosition(const qint64 value)
{
    Q_UNUSED(value);
}

qint64 DummyPlayer::duration() const
{
    return 0;
}

QSizeF DummyPlayer::videoSize() const
{
    return {};
}

qreal DummyPlayer::volume() const
{
    return 0.0;
}

void DummyPlayer::setVolume(const qreal value)
{
    Q_UNUSED(value);
}

bool DummyPlayer::mute() const
{
    return true;
}

void DummyPlayer::setMute(const bool value)
{
    Q_UNUSED(value);
}

bool DummyPlayer::seekable() const
{
    return false;
}

PlaybackState DummyPlayer::playbackState() const
{
    return PlaybackState::Stopped;
}

void DummyPlayer::setPlaybackState(const PlaybackState value)
{
    Q_UNUSED(value);
}

MediaStatus DummyPlayer::mediaStatus() const
{
    return {};
}

LogLevel DummyPlayer::logLevel() const
{
    return LogLevel::Off;
}

void DummyPlayer::setLogLevel(const LogLevel value)
{
    Q_UNUSED(value);
}

qreal DummyPlayer::playbackRate() const
{
    return 1.0;
}

void DummyPlayer::setPlaybackRate(const qreal value)
{
    Q_UNUSED(value);
}

qreal DummyPlayer::aspectRatio() const
{
    return (16.0 / 9.0);
}

void DummyPlayer::setAspectRatio(const qreal value)
{
    Q_UNUSED(value);
}

QUrl DummyPlayer::snapshotDirectory() const
{
    return {};
}

void DummyPlayer::setSnapshotDirectory(const QUrl &value)
{
    Q_UNUSED(value);
}

QString DummyPlayer::snapshotFormat() const
{
    return {};
}

void DummyPlayer::setSnapshotFormat(const QString &value)
{
    Q_UNUSED(value);
}

QString DummyPlayer::snapshotTemplate() const
{
    return {};
}

void DummyPlayer::setSnapshotTemplate(const QString &value)
{
    Q_UNUSED(value);
}

bool DummyPlayer::hardwareDecoding() const
{
    return false;
}

void DummyPlayer::setHardwareDecoding(const bool value)
{
    Q_UNUSED(value);
}

bool DummyPlayer::autoStart() const
{
    return false;
}

void DummyPlayer::setAutoStart(const bool value)
{
    Q_UNUSED(value);
}

bool DummyPlayer::livePreview() const
{
    return false;
}

void DummyPlayer::setLivePreview(const bool value)
{
    Q_UNUSED(value);
}

FillMode DummyPlayer::fillMode() const
{
    return FillMode::PreserveAspectFit;
}

void DummyPlayer::setFillMode(const FillMode value)
{
    Q_UNUSED(value);
}

Chapters DummyPlayer::chapters() const
{
    return {};
}

MetaData DummyPlayer::metaData() const
{
    return {};
}

MediaTracks DummyPlayer::mediaTracks() const
{
    return {};
}

int DummyPlayer::activeVideoTrack() const
{
    return 0;
}

void DummyPlayer::setActiveVideoTrack(const int value)
{
    Q_UNUSED(value);
}

int DummyPlayer::activeAudioTrack() const
{
    return 0;
}

void DummyPlayer::setActiveAudioTrack(const int value)
{
    Q_UNUSED(value);
}

int DummyPlayer::activeSubtitleTrack() const
{
    return 0;
}

void DummyPlayer::setActiveSubtitleTrack(const int value)
{
    Q_UNUSED(value);
}

bool DummyPlayer::rendererReady() const
{
    return false;
}

void DummyPlayer::play()
{
}

void DummyPlayer::pause()
{
}

void DummyPlayer::stop()
{
}

void DummyPlayer::seek(const qint64 value)
{
    Q_UNUSED(value);
}

void DummyPlayer::snapshot()
{
}

bool DummyPlayer::isLoaded() const
{
    return false;
}

bool DummyPlayer::isPlaying() const
{
    return false;
}

bool DummyPlayer::isPaused() const
{
    return false;
}

bool DummyPlayer::isStopped() const
{
    return true;
}

void DummyPlayer::rotateImage(const qreal value)
{
    Q_UNUSED(value);
}

void DummyPlayer::scaleImage(const qreal value)
{
    Q_UNUSED(value);
}

QTMEDIAPLAYER_END_NAMESPACE
