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
#include <QtCore/qlibrary.h>
#include <QtCore/qlibraryinfo.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQMP, "wangwenx190.mediaplayer")

static const char _qmp_backend_dir_envVar[] = "_QTMEDIAPLAYER_BACKEND_SEARCH_PATH";

using RegisterBackendPtr = bool(*)(const char *);
using GetBackendNamePtr = const char *(*)();
using GetBackendVersion = const char *(*)();
using IsRHIBackendSupportedPtr = bool(*)(const int);

struct QMPData
{
public:
    QString searchPath = {};
    QHash<QString, QString> availableBackends = {};

    explicit QMPData()
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        const QString qtPluginsDirPath = QLibraryInfo::path(QLibraryInfo::PluginsPath);
#else
        const QString qtPluginsDirPath = QLibraryInfo::location(QLibraryInfo::PluginsPath);
#endif
        searchPath = qEnvironmentVariable(_qmp_backend_dir_envVar, QDir::toNativeSeparators(qtPluginsDirPath + QStringLiteral("/qtmediaplayer")));
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
            qCWarning(lcQMP) << "Plugin directory" << searchPath << "doesn't exist.";
            return;
        }
        const QFileInfoList entryInfoList = dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Name);
        if (entryInfoList.isEmpty()) {
            qCWarning(lcQMP) << "Plugin directory" << searchPath << "doesn't contain any files.";
            return;
        }
        for (auto &&entryInfo : qAsConst(entryInfoList)) {
            if (!QLibrary::isLibrary(entryInfo.fileName())) {
                continue;
            }
            const auto m_lpBackendName = reinterpret_cast<GetBackendNamePtr>(QLibrary::resolve(entryInfo.canonicalFilePath(), "GetBackendName"));
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
        qCWarning(lcQMP) << value << "is not a directory.";
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

QStringList getAvailableBackends()
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
        qCWarning(lcQMP) << loweredName << "is not an available backend.";
        return false;
    }
    const auto m_lpRegisterBackend = reinterpret_cast<RegisterBackendPtr>(QLibrary::resolve(qmpData()->availableBackends.value(loweredName), "RegisterBackend"));
    if (!m_lpRegisterBackend) {
        qCWarning(lcQMP) << "Failed to resolve \"RegisterBackend()\" from the backend library.";
        return false;
    }
    return m_lpRegisterBackend(qUtf8Printable(loweredName));
}

bool isRHIBackendSupported(const QString &name, const QSGRendererInterface::GraphicsApi api)
{
    Q_ASSERT(!name.isEmpty());
    if (name.isEmpty()) {
        return false;
    }
    const QString loweredName = name.toLower();
    if (!qmpData()->availableBackends.contains(loweredName)) {
        qCWarning(lcQMP) << loweredName << "is not an available backend.";
        return false;
    }
    const auto m_lpIsRHIBackendSupported = reinterpret_cast<IsRHIBackendSupportedPtr>(QLibrary::resolve(qmpData()->availableBackends.value(loweredName), "IsRHIBackendSupported"));
    if (!m_lpIsRHIBackendSupported) {
        qCWarning(lcQMP) << "Failed to resolve \"IsRHIBackendSupported()\" from the backend library.";
        return false;
    }
    return m_lpIsRHIBackendSupported(static_cast<int>(api));
}

QTMEDIAPLAYER_END_NAMESPACE
