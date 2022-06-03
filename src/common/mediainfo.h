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
#include <QtCore/qobject.h>
#include <QtCore/qsize.h>
#include <QtQml/qqml.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

class QTMEDIAPLAYER_COMMON_API MediaInfo : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MediaInfo)
    Q_PROPERTY(QString filePath READ filePath NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString fileName READ fileName NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString fileMimeType READ fileMimeType NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString friendlyFileType READ friendlyFileType NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(qint64 fileSize READ fileSize NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(qint64 duration READ duration NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QSizeF pictureSize READ pictureSize NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString creationDateTime READ creationDateTime NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString modificationDateTime READ modificationDateTime NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString title READ title NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString author READ author NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString album READ album NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString copyright READ copyright NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString rating READ rating NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString location READ location NOTIFY mediaInfoChanged FINAL)
    Q_PROPERTY(QString description READ description NOTIFY mediaInfoChanged FINAL)

public:
    explicit MediaInfo(QObject *parent = nullptr);
    ~MediaInfo() override;

    Q_NODISCARD QString filePath() const;
    Q_NODISCARD QString fileName() const;
    Q_NODISCARD QString fileMimeType() const;
    Q_NODISCARD QString friendlyFileType() const;
    Q_NODISCARD qint64 fileSize() const;
    Q_NODISCARD qint64 duration() const;
    Q_NODISCARD QSizeF pictureSize() const;
    Q_NODISCARD QString creationDateTime() const;
    Q_NODISCARD QString modificationDateTime() const;
    Q_NODISCARD QString title() const;
    Q_NODISCARD QString author() const;
    Q_NODISCARD QString album() const;
    Q_NODISCARD QString copyright() const;
    Q_NODISCARD QString rating() const;
    Q_NODISCARD QString location() const;
    Q_NODISCARD QString description() const;

private Q_SLOTS:
    void reset();

Q_SIGNALS:
    void mediaInfoChanged();

private:
    friend class MediaPlayer;

    QString m_filePath = {};
    QString m_fileName = {};
    QString m_fileMimeType = {};
    QString m_friendlyFileType = {};
    qint64 m_fileSize = 0;
    qint64 m_duration = 0;
    QSizeF m_pictureSize = {};
    QString m_creationDateTime = {};
    QString m_modificationDateTime = {};
    QString m_title = {};
    QString m_author = {};
    QString m_album = {};
    QString m_copyright = {};
    QString m_rating = {};
    QString m_location = {};
    QString m_description = {};
};

QTMEDIAPLAYER_END_NAMESPACE

Q_DECLARE_METATYPE(QTMEDIAPLAYER_PREPEND_NAMESPACE(MediaInfo))
QML_DECLARE_TYPE(QTMEDIAPLAYER_PREPEND_NAMESPACE(MediaInfo))
