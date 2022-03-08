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

#include "common_global.h"
#include <QtCore/qlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

Q_NAMESPACE_EXPORT(QTMEDIAPLAYER_COMMON_API)

enum class PlaybackState : int
{
    Stopped = 0,
    Playing = 1,
    Paused = 2
};
Q_ENUM_NS(PlaybackState)

enum class MediaStatusFlag : int
{
    Invalid = -1,
    NoMedia = 0,
    Unloaded = 1,
    Loading = 2,
    Loaded = 3,
    Prepared = 4,
    Stalled = 5,
    Buffering = 6,
    Buffered = 7,
    End = 8,
    Seeking = 9
};
Q_DECLARE_FLAGS(MediaStatus, MediaStatusFlag)
Q_FLAG_NS(MediaStatus)

enum class LogLevel : int
{
    Off = 0,
    Info = 1,
    Debug = 2,
    Warning = 3,
    Critical = 4,
    Fatal = 5
};
Q_ENUM_NS(LogLevel)

enum class FillMode : int
{
    PreserveAspectFit = 0,
    PreserveAspectCrop = 1,
    Stretch = 2
};
Q_ENUM_NS(FillMode)

struct ChapterInfo
{
    QString title = {};
    qint64 startTime = 0;
    qint64 endTime = 0;
};

struct MediaTracks
{
    QList<QVariantHash> video = {};
    QList<QVariantHash> audio = {};
    QList<QVariantHash> subtitle = {};
};

using Chapters = QList<ChapterInfo>;

using MetaData = QVariantHash;

QTMEDIAPLAYER_END_NAMESPACE

Q_DECLARE_OPERATORS_FOR_FLAGS(QTMEDIAPLAYER_PREPEND_NAMESPACE(MediaStatus))
Q_DECLARE_METATYPE(QTMEDIAPLAYER_PREPEND_NAMESPACE(ChapterInfo))
Q_DECLARE_METATYPE(QTMEDIAPLAYER_PREPEND_NAMESPACE(MediaTracks))
Q_DECLARE_METATYPE(QTMEDIAPLAYER_PREPEND_NAMESPACE(Chapters))
Q_DECLARE_METATYPE(QTMEDIAPLAYER_PREPEND_NAMESPACE(MetaData))
