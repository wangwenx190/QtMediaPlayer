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

#include <QtCore/qobject.h>
#include <QtCore/qmap.h>
#include <QtCore/qurl.h>
#include <QtQml/qqmlregistration.h>

class History : public QObject
{
    Q_OBJECT
#ifdef QML_ELEMENT
    QML_ELEMENT
#endif
#ifdef QML_SINGLETON
    QML_SINGLETON
#endif
    Q_DISABLE_COPY_MOVE(History)

public:
    explicit History(QObject *parent = nullptr);
    ~History() override;

public Q_SLOTS:
    [[nodiscard]] qint64 getLastPosition(const QUrl &url) const;
    void insertNewRecord(const QUrl &url, const qint64 pos);
    void clear();

private:
    void reload();
    void save();

private:
    QMap<QUrl, qint64> m_history = {};
};
