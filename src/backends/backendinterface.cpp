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

#include "backendinterface.h"
#include <QtCore/qdebug.h>
#include <QtCore/qmimedatabase.h>
#include <QtCore/qmimetype.h>
#include <QtCore/qdatetime.h>

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QTMEDIAPLAYER_PREPEND_NAMESPACE(MediaPlayer)::Chapters &chapters)
{
    QDebugStateSaver saver(d);
    d.nospace();
    d.noquote();
    QString str = {};
    for (auto &&chapter : qAsConst(chapters)) {
        str.append(QStringLiteral("(title: %1, startTime: %2)").arg(chapter.title, QString::number(chapter.startTime)));
    }
    d << "QList(" << str << ')';
    return d;
}

QDebug operator<<(QDebug d, const QTMEDIAPLAYER_PREPEND_NAMESPACE(MediaPlayer)::MediaTracks &tracks)
{
    QDebugStateSaver saver(d);
    d.nospace();
    d.noquote();
    d << "MediaPlayer::MediaTracks(Video tracks:" << tracks.video << "; Audio tracks:" << tracks.audio << "; Subtitle tracks:" << tracks.sub << ')';
    return d;
}
#endif

static inline QStringList suffixesToMimeTypes(const QStringList &suffixes)
{
    QStringList mimeTypes = {};
    const QMimeDatabase db;
    for (auto &&suffix : qAsConst(suffixes)) {
        const QList<QMimeType> typeList = db.mimeTypesForFileName(suffix);
        if (!typeList.isEmpty()) {
            for (auto &&mimeType : qAsConst(typeList)) {
                const QString name = mimeType.name();
                if (!name.isEmpty()) {
                    mimeTypes.append(name);
                }
            }
        }
    }
    if (!mimeTypes.isEmpty()) {
        mimeTypes.removeDuplicates();
    }
    return mimeTypes;
}

QTMEDIAPLAYER_BEGIN_NAMESPACE

MediaPlayer::MediaPlayer(QQuickItem *parent) : QQuickItem(parent)
{
    // Without this flag, our item won't paint anything.
    setFlag(ItemHasContents, true);

    qRegisterMetaType<ChapterInfo>();
    qRegisterMetaType<Chapters>();
    qRegisterMetaType<MetaData>();
    qRegisterMetaType<MediaTracks>();
}

MediaPlayer::~MediaPlayer() = default;

QStringList MediaPlayer::videoMimeTypes()
{
    return suffixesToMimeTypes(videoSuffixes());
}

QStringList MediaPlayer::audioMimeTypes()
{
    return suffixesToMimeTypes(audioSuffixes());
}

QString MediaPlayer::formatTime(const qint64 ms, const QString &pattern) const
{
    Q_ASSERT(ms >= 0);
    Q_ASSERT(!pattern.isEmpty());
    if ((ms < 0) || pattern.isEmpty()) {
        return {};
    }
    return QTime(0, 0).addMSecs(ms).toString(pattern);
}

QTMEDIAPLAYER_END_NAMESPACE
