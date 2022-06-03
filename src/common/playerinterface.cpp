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

#include "playerinterface.h"
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qmimedatabase.h>
#include <QtCore/qmimetype.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qfileinfo.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qscreen.h>
#include <QtQuick/qquickwindow.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

static const QString kTitle = QStringLiteral("title");
static const QString kAuthor = QStringLiteral("author");
static const QString kAlbum = QStringLiteral("album");
static const QString kCopyright = QStringLiteral("copyright");
static const QString kRating = QStringLiteral("rating");
static const QString kDescription = QStringLiteral("description");

#ifndef QT_NO_DEBUG_STREAM
[[nodiscard]] QDebug operator<<(QDebug d, const ChapterInfo &info)
{
    const QDebugStateSaver saver(d);
    d.nospace();
    d.noquote();
    d << "MediaPlayer::ChapterInfo(title: " << info.title
      << ", startTime: " << info.startTime
      << ", endTime: " << info.endTime << ')';
    return d;
}

[[nodiscard]] QDebug operator<<(QDebug d, const MediaTracks &tracks)
{
    const QDebugStateSaver saver(d);
    d.nospace();
    d.noquote();
    d << "MediaPlayer::MediaTracks(Video tracks:" << tracks.video
      << "; Audio tracks:" << tracks.audio
      << "; Subtitle tracks:" << tracks.subtitle << ')';
    return d;
}
#endif

[[nodiscard]] static inline QStringList suffixesToMimeTypes(const QStringList &suffixes)
{
    if (suffixes.isEmpty()) {
        return {};
    }
    QStringList mimeTypes = {};
    const QMimeDatabase mimeDb = {};
    for (auto &&suffix : qAsConst(suffixes)) {
        const QList<QMimeType> typeList = mimeDb.mimeTypesForFileName(suffix);
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

[[nodiscard]] static inline QScreen *getCurrentScreen(const QQuickWindow * const window)
{
    Q_ASSERT(window);
    if (!window) {
        return nullptr;
    }
    QScreen *screen = window->screen();
    if (screen) {
        return screen;
    }
    return QGuiApplication::primaryScreen();
}

MediaPlayer::MediaPlayer(QQuickItem *parent) : QQuickItem(parent)
{
    // Without this flag, our item won't draw anything. It must be set.
    setFlag(ItemHasContents);

    m_mediaInfo.reset(new MediaInfo(this));

    // Re-calculate the recommended window size and position everytime when the videoSize changes.
    connect(this, &MediaPlayer::videoSizeChanged, this, &MediaPlayer::recommendedWindowSizeChanged);
    connect(this, &MediaPlayer::recommendedWindowSizeChanged, this, &MediaPlayer::recommendedWindowPositionChanged);

    connect(this, &MediaPlayer::mediaTracksChanged, this, [this](){
        m_mediaInfo->reset();

        if (!isStopped()) {
            const QString path = filePath();
            if (!path.isEmpty() && QFileInfo::exists(path)) {
                const QFileInfo fileInfo(path);
                m_mediaInfo->m_filePath = QDir::toNativeSeparators(fileInfo.canonicalFilePath());
                m_mediaInfo->m_fileName = fileInfo.fileName();
                m_mediaInfo->m_fileSize = fileInfo.size();
                m_mediaInfo->m_creationDateTime = fileInfo.fileTime(QFile::FileBirthTime).toString();
                m_mediaInfo->m_modificationDateTime = fileInfo.fileTime(QFile::FileModificationTime).toString();
                m_mediaInfo->m_location = QDir::toNativeSeparators(fileInfo.canonicalPath());

                const QMimeDatabase mimeDb = {};
                const QMimeType mime = mimeDb.mimeTypeForFile(fileInfo);
                if (mime.isValid()) {
                    m_mediaInfo->m_fileMimeType = mime.name();
                    m_mediaInfo->m_friendlyFileType = mime.comment();
                }
            }

            m_mediaInfo->m_duration = duration();
            m_mediaInfo->m_pictureSize = videoSize();

            const MetaData md = metaData();
            if (!md.isEmpty()) {
                m_mediaInfo->m_title = md.value(kTitle).toString();
                m_mediaInfo->m_author = md.value(kAuthor).toString();
                m_mediaInfo->m_album = md.value(kAlbum).toString();
                m_mediaInfo->m_copyright = md.value(kCopyright).toString();
                m_mediaInfo->m_rating = md.value(kRating).toString();
                m_mediaInfo->m_description = md.value(kDescription).toString();
            }
        }

        Q_EMIT hasVideoChanged();
        Q_EMIT hasAudioChanged();
        Q_EMIT hasSubtitleChanged();

        Q_EMIT m_mediaInfo->mediaInfoChanged();
    });
}

MediaPlayer::~MediaPlayer() = default;

QString MediaPlayer::graphicsApiName() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    switch (QQuickWindow::graphicsApi()) {
    case QSGRendererInterface::Direct3D11:
        return QStringLiteral("Direct3D11");
    case QSGRendererInterface::Vulkan:
        return QStringLiteral("Vulkan");
    case QSGRendererInterface::Metal:
        return QStringLiteral("Metal");
    case QSGRendererInterface::OpenGL:
        return QStringLiteral("OpenGL");
    case QSGRendererInterface::Software:
        return QStringLiteral("Software");
    case QSGRendererInterface::Null:
        return QStringLiteral("Null");
    default:
        return QStringLiteral("Unknown");
    }
#else
    return QQuickWindow::sceneGraphBackend();
#endif
}

QStringList MediaPlayer::videoFileMimeTypes()
{
    return suffixesToMimeTypes(videoFileSuffixes());
}

QStringList MediaPlayer::audioFileMimeTypes()
{
    return suffixesToMimeTypes(audioFileSuffixes());
}

QString MediaPlayer::formatTime(const qint64 ms, const QString &pattern)
{
    Q_ASSERT(ms >= 0);
    Q_ASSERT(!pattern.isEmpty());
    if ((ms < 0) || pattern.isEmpty()) {
        return {};
    }
    return QTime(0, 0).addMSecs(ms).toString(pattern);
}

bool MediaPlayer::isVideoFile(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());
    if (fileName.isEmpty()) {
        return false;
    }
    // No need to check whether it exists or not, whether it's
    // a file or a directory, we are just interested in the
    // filename string, all these things do not matter.
    const QString suffix = QFileInfo(fileName).suffix(); // Don't use "QFileInfo::completeSuffix()".
    if (suffix.isEmpty()) {
        return false;
    }
    return videoFileSuffixes().contains(QStringLiteral("*.") + suffix, Qt::CaseInsensitive);
}

bool MediaPlayer::isAudioFile(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());
    if (fileName.isEmpty()) {
        return false;
    }
    const QString suffix = QFileInfo(fileName).suffix();
    if (suffix.isEmpty()) {
        return false;
    }
    return audioFileSuffixes().contains(QStringLiteral("*.") + suffix, Qt::CaseInsensitive);
}

bool MediaPlayer::isSubtitleFile(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());
    if (fileName.isEmpty()) {
        return false;
    }
    const QString suffix = QFileInfo(fileName).suffix();
    if (suffix.isEmpty()) {
        return false;
    }
    return subtitleFileSuffixes().contains(QStringLiteral("*.") + suffix, Qt::CaseInsensitive);
}

bool MediaPlayer::isMediaFile(const QString &fileName)
{
    Q_ASSERT(!fileName.isEmpty());
    if (fileName.isEmpty()) {
        return false;
    }
    return (isVideoFile(fileName) || isAudioFile(fileName));
}

bool MediaPlayer::isPlayingVideo() const
{
    if (isStopped()) {
        return false;
    }
    return isVideoFile(fileName());
}

bool MediaPlayer::isPlayingAudio() const
{
    if (isStopped()) {
        return false;
    }
    return isAudioFile(fileName());
}

QSizeF MediaPlayer::recommendedWindowSize() const
{
    const QSizeF pictureSize = videoSize();
    // Well, if we can't get a valid picture size, there's no need to
    // continue run all the logic below.
    if (pictureSize.isEmpty()) {
        return {};
    }
    const QQuickWindow * const win = window();
    Q_ASSERT(win);
    if (!win) {
        return {};
    }
    // Only calculate the recommended size when the window is in the normal state.
    if (win->visibility() != QQuickWindow::Windowed) {
        return {};
    }
    const QScreen * const screen = getCurrentScreen(win);
    if (!screen) {
        return {};
    }
    const QSize screenSize = screen->availableSize();
    qreal zoomFactor = 1.0;
    const bool widthGreater = pictureSize.width() > screenSize.width();
    const bool heightGreater = pictureSize.height() > screenSize.height();
    if (widthGreater || heightGreater) {
        bool useWidth = false;
        if (widthGreater && heightGreater) {
            if (pictureSize.width() >= pictureSize.height()) {
                useWidth = true;
            }
        } else if (widthGreater) {
            useWidth = true;
        }
        zoomFactor = (useWidth
                          ? (qreal(screenSize.width()) / qreal(pictureSize.width()))
                          : (qreal(screenSize.height()) / qreal(pictureSize.height())));
    }
    // Fit the current screen as much as possible.
    return QSizeF(pictureSize * zoomFactor);
}

QPointF MediaPlayer::recommendedWindowPosition() const
{
    const QQuickWindow * const win = window();
    Q_ASSERT(win);
    if (!win) {
        return {};
    }
    // Only calculate the recommended position when the window is in the normal state.
    if (win->visibility() != QQuickWindow::Windowed) {
        return {};
    }
    const QScreen * const screen = getCurrentScreen(win);
    if (!screen) {
        return {};
    }
    // Use QScreen::availableSize() to take the taskbar into account.
    const QSize screenSize = screen->availableSize();
    const qreal newX = qreal(screenSize.width() - win->width()) / 2.0;
    const qreal newY = qreal(screenSize.height() - win->height()) / 2.0;
    // This offset is needed in case the user put the taskbar on the top or left side.
    const QPoint offset = screen->availableGeometry().topLeft();
    return QPointF(newX + offset.x(), newY + offset.y());
}

bool MediaPlayer::hasVideo() const
{
    return (isStopped() ? false : !mediaTracks().video.isEmpty());
}

bool MediaPlayer::hasAudio() const
{
    return (isStopped() ? false : !mediaTracks().audio.isEmpty());
}

bool MediaPlayer::hasSubtitle() const
{
    return (isStopped() ? false : !mediaTracks().subtitle.isEmpty());
}

MediaInfo *MediaPlayer::mediaInfo() const
{
    return m_mediaInfo.data();
}

void MediaPlayer::play(const QUrl &url)
{
    Q_ASSERT(url.isValid());
    if (!url.isValid()) {
        return;
    }
    setSource(url);
    play(); // Start playing regardless of the value of AutoStart.
}

void MediaPlayer::open(const QUrl &url)
{
    Q_ASSERT(url.isValid());
    if (!url.isValid()) {
        return;
    }
    play(url);
}

void MediaPlayer::nextChapter()
{
    if (isStopped()) {
        return;
    }
    const Chapters ch = chapters();
    if (ch.isEmpty()) {
        return;
    }
    const int total = ch.count();
    const qint64 pos = position();
    // "total - 2": Nothing to do if we are in the last chapter.
    for (int i = (total - 2); i >= 0; --i) {
        // Don't reply on endTime, it's not reliable for some backends.
        if (pos < ch.at(i).startTime) {
            continue;
        }
        const int nextChapterNo = i + 1;
        if (nextChapterNo <= (total - 1)) {
            seek(ch.at(nextChapterNo).startTime);
        }
    }
}

void MediaPlayer::previousChapter()
{
    if (isStopped()) {
        return;
    }
    const Chapters ch = chapters();
    if (ch.isEmpty()) {
        return;
    }
    const int total = ch.count();
    const qint64 pos = position();
    // "i != 0": Nothing to do if we are in the first chapter.
    for (int i = (total - 1); i != 0; --i) {
        // Don't reply on endTime, it's not reliable for some backends.
        if (pos < ch.at(i).startTime) {
            continue;
        }
        const int previousChapterNo = i - 1;
        if (previousChapterNo >= 0) {
            seek(ch.at(previousChapterNo).startTime);
        }
    }
}

QTMEDIAPLAYER_END_NAMESPACE
