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

#include "qtmediaplayer.h"
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qlibrary.h>

static const char _qmp_backend_dir_envVar[] = "_QTMEDIAPLAYER_BACKEND_SEARCH_PATH";

QTMEDIAPLAYER_BEGIN_NAMESPACE

using RegisterBackendPtr = bool(*)(const char *);
using BackendNamePtr = const char *(*)();

struct QMPData
{
public:
    QString searchPath = {};
    QHash<QString, QString> availableBackends = {};

    explicit QMPData()
    {
        searchPath = qEnvironmentVariable(_qmp_backend_dir_envVar, QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + QStringLiteral("/QtMediaPlayerBackends")));
        refreshCache();
    }

    void refreshCache()
    {
        Q_ASSERT(!searchPath.isEmpty());
        if (searchPath.isEmpty()) {
            return;
        }
        if (!availableBackends.isEmpty()) {
            availableBackends.clear();
        }
        const QDir dir(searchPath);
        if (!dir.exists()) {
            qWarning() << "Plugin directory" << searchPath << "doesn't exist.";
            return;
        }
        const QFileInfoList entryInfoList = dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);
        if (entryInfoList.isEmpty()) {
            qWarning() << "Plugin directory" << searchPath << "doesn't contain any files.";
            return;
        }
        for (auto &&entryInfo : qAsConst(entryInfoList)) {
            if (!QLibrary::isLibrary(entryInfo.fileName())) {
                continue;
            }
            const auto m_lpBackendName = reinterpret_cast<BackendNamePtr>(QLibrary::resolve(entryInfo.canonicalFilePath(), "GetBackendName"));
            if (!m_lpBackendName) {
                continue;
            }
            const QString backendName = QString::fromUtf8(m_lpBackendName()).toLower();
            Q_ASSERT(!backendName.isEmpty());
            if (backendName.isEmpty()) {
                continue;
            }
            Q_ASSERT(!availableBackends.contains(backendName));
            if (availableBackends.contains(backendName)) {
                continue;
            }
            availableBackends.insert(backendName, QDir::toNativeSeparators(entryInfo.canonicalFilePath()));
        }
    }
};

Q_GLOBAL_STATIC(QMPData, qmpData)

void setPluginSearchPath(const QString &value)
{
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return;
    }
    if (!QFileInfo(value).isDir()) {
        qWarning() << value << "is not a directory.";
        return;
    }
    qmpData()->searchPath = QDir::toNativeSeparators(value);
    if (qmpData()->searchPath.endsWith(u'\\') || qmpData()->searchPath.endsWith(u'/')) {
        qmpData()->searchPath.chop(1);
    }
    qmpData()->refreshCache();
}

QString getPluginSearchPath()
{
    return qmpData()->searchPath;
}

QStringList availableBackends()
{
    QStringList list = {};
    for (auto &&backendName : qAsConst(qmpData()->availableBackends)) {
        list.append(backendName);
    }
    return list;
}

bool initializeBackend(const QString &value)
{
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return false;
    }
    const QString loweredName = value.toLower();
    if (!qmpData()->availableBackends.contains(loweredName)) {
        qWarning() << loweredName << "is not an available backend.";
        return false;
    }
    const auto m_lpRegisterBackend = reinterpret_cast<RegisterBackendPtr>(QLibrary::resolve(qmpData()->availableBackends.value(loweredName), "RegisterBackend"));
    if (!m_lpRegisterBackend) {
        qWarning() << "Failed to resolve \"RegisterBackend()\" from the backend library.";
        return false;
    }
    return m_lpRegisterBackend(qUtf8Printable(loweredName));
}

QTMEDIAPLAYER_END_NAMESPACE
