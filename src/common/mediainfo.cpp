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

#include "mediainfo.h"

QTMEDIAPLAYER_BEGIN_NAMESPACE

MediaInfo::MediaInfo(QObject *parent) : QObject(parent)
{
}

MediaInfo::~MediaInfo() = default;

QString MediaInfo::filePath() const
{
    return m_filePath;
}

QString MediaInfo::fileName() const
{
    return m_fileName;
}

QString MediaInfo::fileMimeType() const
{
    return m_fileMimeType;
}

QString MediaInfo::friendlyFileType() const
{
    return m_friendlyFileType;
}

qint64 MediaInfo::fileSize() const
{
    return m_fileSize;
}

qint64 MediaInfo::duration() const
{
    return m_duration;
}

QSizeF MediaInfo::pictureSize() const
{
    return m_pictureSize;
}

QString MediaInfo::creationDateTime() const
{
    return m_creationDateTime;
}

QString MediaInfo::modificationDateTime() const
{
    return m_modificationDateTime;
}

QString MediaInfo::title() const
{
    return m_title;
}

QString MediaInfo::author() const
{
    return m_author;
}

QString MediaInfo::album() const
{
    return m_album;
}

QString MediaInfo::copyright() const
{
    return m_copyright;
}

QString MediaInfo::rating() const
{
    return m_rating;
}

QString MediaInfo::location() const
{
    return m_location;
}

QString MediaInfo::description() const
{
    return m_description;
}

void MediaInfo::reset()
{
    m_filePath.clear();
    m_fileName.clear();
    m_fileMimeType.clear();
    m_friendlyFileType.clear();
    m_fileSize = 0;
    m_duration = 0;
    m_pictureSize = {};
    m_creationDateTime.clear();
    m_modificationDateTime.clear();
    m_title.clear();
    m_author.clear();
    m_album.clear();
    m_copyright.clear();
    m_rating.clear();
    m_location.clear();
    m_description.clear();

    Q_EMIT mediaInfoChanged();
}

QTMEDIAPLAYER_END_NAMESPACE
