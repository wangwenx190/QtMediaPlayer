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

#include "history.h"
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qsavefile.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>

static const QString kUrl = QStringLiteral("url");
static const QString kPosition = QStringLiteral("position");

History::History(QObject *parent) : QObject(parent)
{
    reload();
}

History::~History()
{
    save();
}

qint64 History::getLastPosition(const QUrl &url) const
{
    if (!url.isValid()) {
        return 0;
    }
    if (m_history.isEmpty()) {
        return 0;
    }
    if (!m_history.contains(url)) {
        return 0;
    }
    return qMax(m_history.value(url), qint64(0));
}

void History::insertNewRecord(const QUrl &url, const qint64 pos)
{
    if (!url.isValid() || (pos < 0)) {
        return;
    }
    if (!m_history.isEmpty()) {
        if (m_history.contains(url)) {
            m_history.remove(url);
        }
    }
    m_history.insert(m_history.constBegin(), url, pos);
}

void History::clear()
{
    if (!m_history.isEmpty()) {
        m_history.clear();
    }
}

void History::reload()
{
    static const QString jsonFilePath = QCoreApplication::applicationDirPath() + QStringLiteral("/history.json");
    QFile file(jsonFilePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }
    const QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isEmpty()) {
        return;
    }
    if (!doc.isArray()) {
        return;
    }
    const QJsonArray array = doc.array();
    if (array.isEmpty()) {
        return;
    }
    if (!m_history.isEmpty()) {
        m_history.clear();
    }
    for (auto &&arrayValue : qAsConst(array)) {
        const QJsonObject object = arrayValue.toObject();
        const QUrl url = QUrl::fromUserInput(object.value(kUrl).toString(),
                                             QCoreApplication::applicationDirPath(), QUrl::AssumeLocalFile);
        const qint64 position = qMax(qint64(object.value(kPosition).toDouble()), qint64(0));
        m_history.insert(url, position);
    }
}

void History::save()
{
    if (m_history.isEmpty()) {
        return;
    }
    static const QString jsonFilePath = QCoreApplication::applicationDirPath() + QStringLiteral("/history.json");
    QSaveFile file(jsonFilePath);
    if (!file.open(QSaveFile::WriteOnly | QSaveFile::Text | QSaveFile::Truncate)) {
        return;
    }
    QJsonArray array = {};
    auto it = m_history.constBegin();
    while (it != m_history.constEnd()) {
        QJsonObject object = {};
        const QUrl source = it.key();
        const QString url = source.isLocalFile() ?
                                QDir::toNativeSeparators(source.toLocalFile()) : source.toString();
        object.insert(kUrl, url);
        object.insert(kPosition, qMax(it.value(), qint64(0)));
        array.append(object);
        ++it;
    }
    file.write(QJsonDocument(array).toJson());
    file.commit();
}
