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

#include "qtmediaplayer.h"
#include <QtCore/qdebug.h>
#include <QtCore/qmutex.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qlibraryinfo.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQMP, "wangwenx190.mediaplayer")

static constexpr const char _qmp_backend_dir_envVar[] = "QTMEDIAPLAYER_BACKEND_SEARCH_PATH";

using RegisterBackendPtr = bool(*)(const char *);
using GetBackendNamePtr = const char *(*)();
using GetBackendVersion = const char *(*)();
using IsRHIBackendSupportedPtr = bool(*)(const int);
using FreeStringPtr = void(*)(const char *);

struct QMPData
{
    QMutex mutex = {};

    QStringList searchPaths = {};
    QHash<QString, QString> availableBackends = {};

    explicit QMPData()
    {
        init();
    }

    ~QMPData() = default;

    inline void init()
    {
        static bool inited = false;
        if (inited) {
            return;
        }
        inited = true;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        static const QString qtPluginsDirPath = QLibraryInfo::path(QLibraryInfo::PluginsPath);
#else
        static const QString qtPluginsDirPath = QLibraryInfo::location(QLibraryInfo::PluginsPath);
#endif
        if (!qtPluginsDirPath.isEmpty()) {
            mutex.lock();
            searchPaths << QDir::toNativeSeparators(qtPluginsDirPath + QStringLiteral("/qtmediaplayer"));
            mutex.unlock();
        }
        const QString rawPathsFromEnvVar = qEnvironmentVariable(_qmp_backend_dir_envVar);
        if (!rawPathsFromEnvVar.isEmpty()) {
            const QStringList paths = rawPathsFromEnvVar.split(u';', Qt::SkipEmptyParts, Qt::CaseInsensitive);
            if (!paths.isEmpty()) {
                mutex.lock();
                searchPaths << paths;
                mutex.unlock();
            }
        }
        refreshCache();
    }

    inline void loadFromDir(const QString &path)
    {
        Q_ASSERT(!path.isEmpty());
        if (path.isEmpty()) {
            return;
        }
        const QDir dir(path);
        if (!dir.exists()) {
            qCWarning(lcQMP) << "Plugin directory" << path << "doesn't exist.";
            return;
        }
        const QFileInfoList entryInfoList = dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Readable);
        if (entryInfoList.isEmpty()) {
            qCWarning(lcQMP) << "Plugin directory" << path << "doesn't contain any files.";
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
            mutex.lock();
            Q_ASSERT(!availableBackends.contains(backendName));
            if (availableBackends.contains(backendName)) {
                mutex.unlock();
                continue;
            }
            availableBackends.insert(backendName, QDir::toNativeSeparators(entryInfo.canonicalFilePath()));
            mutex.unlock();
        }
    }

    inline void refreshCache()
    {
        mutex.lock();
        Q_ASSERT(!searchPaths.isEmpty());
        if (searchPaths.isEmpty()) {
            mutex.unlock();
            return;
        }
        if (!availableBackends.isEmpty()) {
            availableBackends.clear();
        }
        const QStringList paths = searchPaths;
        mutex.unlock();
        for (auto &&path : qAsConst(paths)) {
            if (!path.isEmpty()) {
                loadFromDir(path);
            }
        }
    }

    inline void addSearchDir(const QString &path)
    {
        Q_ASSERT(!path.isEmpty());
        if (path.isEmpty()) {
            return;
        }
        const QFileInfo fileInfo(path);
        if (!fileInfo.exists()) {
            qCWarning(lcQMP) << path << "doesn't exist.";
            return;
        }
        if (!fileInfo.isDir()) {
            qCWarning(lcQMP) << path << "is not a directory.";
            return;
        }
        const QString cleanPath = QDir::toNativeSeparators(fileInfo.canonicalFilePath());
        mutex.lock();
        if (searchPaths.contains(cleanPath)) {
            mutex.unlock();
            return;
        }
        searchPaths << cleanPath;
        mutex.unlock();
        loadFromDir(cleanPath);
    }

private:
    Q_DISABLE_COPY_MOVE(QMPData)
};

Q_GLOBAL_STATIC(QMPData, qmpData)

void addPluginSearchPath(const QString &value)
{
    qmpData()->addSearchDir(value);
}

QStringList getPluginSearchPaths()
{
    QMutexLocker locker(&qmpData()->mutex);
    return qmpData()->searchPaths;
}

QStringList getAvailableBackends()
{
    QMutexLocker locker(&qmpData()->mutex);
    if (qmpData()->availableBackends.isEmpty()) {
        return {};
    }
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
    QMutexLocker locker(&qmpData()->mutex);
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
    QMutexLocker locker(&qmpData()->mutex);
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
