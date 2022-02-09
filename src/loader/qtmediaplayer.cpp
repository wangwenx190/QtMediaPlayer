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
#include "../common/backendinterface.h"

QTMEDIAPLAYER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQMPLoader, "wangwenx190.qtmediaplayer.loader")

static constexpr const char _qmp_backend_dir_envVar[] = "QTMEDIAPLAYER_BACKEND_SEARCH_PATH";

struct QMPData
{
    QMutex m_mutex = {};

    QStringList m_searchPaths = {};
    QMap<QString, QMPBackend *> m_availableBackends = {};

    explicit QMPData()
    {
        init();
    }

    ~QMPData()
    {
        QMutexLocker locker(&m_mutex);
        if (!m_availableBackends.isEmpty()) {
            for (auto &&backend : qAsConst(m_availableBackends)) {
                if (backend) {
                    backend->Release();
                }
            }
            m_availableBackends.clear();
        }
    }

    inline void init()
    {
        static bool inited = false;
        if (inited) {
            return;
        }
        inited = true;
        bool searchPathsNotEmpty = false;
        const QString rawPathsFromEnvVar = qEnvironmentVariable(_qmp_backend_dir_envVar);
        if (!rawPathsFromEnvVar.isEmpty()) {
            const QStringList paths = rawPathsFromEnvVar.split(u';', Qt::SkipEmptyParts, Qt::CaseInsensitive);
            if (!paths.isEmpty()) {
                m_mutex.lock();
                m_searchPaths << paths;
                m_mutex.unlock();
                searchPathsNotEmpty = true;
            }
        }
        if (searchPathsNotEmpty) {
            refreshCache();
        }
    }

    inline void loadFromDir(const QString &path)
    {
        Q_ASSERT(!path.isEmpty());
        if (path.isEmpty()) {
            return;
        }
        const QDir dir(path);
        if (!dir.exists()) {
            qCWarning(lcQMPLoader) << "Plugin directory" << path << "doesn't exist.";
            return;
        }
        const QFileInfoList entryInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
        if (entryInfoList.isEmpty()) {
            qCWarning(lcQMPLoader) << "Plugin directory" << path << "doesn't contain any files.";
            return;
        }
        for (auto &&entryInfo : qAsConst(entryInfoList)) {
            if (!QLibrary::isLibrary(entryInfo.fileName())) {
                continue;
            }
            const QString libraryPath = entryInfo.canonicalFilePath();
            const auto m_lpQueryBackend = reinterpret_cast<bool(*)(QMPBackend **)>(
                                              QLibrary::resolve(libraryPath, "QueryBackend"));
            if (!m_lpQueryBackend) {
                continue;
            }
            QMPBackend *backend = nullptr;
            if (!m_lpQueryBackend(&backend)) {
                qCWarning(lcQMPLoader) << "Failed to create backend instance for" << libraryPath
                                       << ". This should not happen.";
                continue;
            }
            Q_CHECK_PTR(backend);
            if (!backend) {
                qCWarning(lcQMPLoader) << "A null pointer is returned for" << libraryPath
                                       << ". This is very wrong.";
                continue;
            }
            const QString backendName = backend->name();
            if (!backend->available()) {
                backend->Release();
                backend = nullptr;
                qCWarning(lcQMPLoader) << "The player backend" << backendName
                                       << "is not available. Please check the requirements.";
                continue;
            }
            m_mutex.lock();
            Q_ASSERT(!m_availableBackends.contains(backendName));
            if (m_availableBackends.contains(backendName)) {
                m_mutex.unlock();
                continue;
            }
            m_availableBackends.insert(backendName.toLower(), backend);
            m_mutex.unlock();
        }
    }

    inline void refreshCache()
    {
        m_mutex.lock();
        Q_ASSERT(!m_searchPaths.isEmpty());
        if (m_searchPaths.isEmpty()) {
            m_mutex.unlock();
            return;
        }
        if (!m_availableBackends.isEmpty()) {
            m_availableBackends.clear();
        }
        const QStringList paths = m_searchPaths;
        m_mutex.unlock();
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
            qCWarning(lcQMPLoader) << path << "doesn't exist.";
            return;
        }
        if (!fileInfo.isDir()) {
            qCWarning(lcQMPLoader) << path << "is not a directory.";
            return;
        }
        const QString cleanPath = QDir::toNativeSeparators(fileInfo.canonicalFilePath());
        m_mutex.lock();
        if (m_searchPaths.contains(cleanPath)) {
            m_mutex.unlock();
            return;
        }
        m_searchPaths << cleanPath;
        m_mutex.unlock();
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
    QMutexLocker locker(&qmpData()->m_mutex);
    return qmpData()->m_searchPaths;
}

QStringList getAvailableBackends()
{
    QMutexLocker locker(&qmpData()->m_mutex);
    if (qmpData()->m_availableBackends.isEmpty()) {
        return {};
    }
    QStringList list = {};
    auto it = qmpData()->m_availableBackends.constBegin();
    while (it != qmpData()->m_availableBackends.constEnd()) {
        list << it.key();
        ++it;
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
    QMutexLocker locker(&qmpData()->m_mutex);
    if (!qmpData()->m_availableBackends.contains(loweredName)) {
        qCWarning(lcQMPLoader) << loweredName << "is not an available backend.";
        return false;
    }
    const auto backend = qmpData()->m_availableBackends.value(loweredName);
    Q_ASSERT(backend);
    if (!backend) {
        qCWarning(lcQMPLoader) << "A null pointer is returned, this is very wrong.";
        return false;
    }
    return backend->initialize();
}

bool isRHIBackendSupported(const QString &name, const QSGRendererInterface::GraphicsApi api)
{
    Q_ASSERT(!name.isEmpty());
    if (name.isEmpty()) {
        return false;
    }
    const QString loweredName = name.toLower();
    QMutexLocker locker(&qmpData()->m_mutex);
    if (!qmpData()->m_availableBackends.contains(loweredName)) {
        qCWarning(lcQMPLoader) << loweredName << "is not an available backend.";
        return false;
    }
    const auto backend = qmpData()->m_availableBackends.value(loweredName);
    Q_ASSERT(backend);
    if (!backend) {
        qCWarning(lcQMPLoader) << "A null pointer is returned, this is very wrong.";
        return false;
    }
    return backend->isRHIBackendSupported(api);
}

QTMEDIAPLAYER_END_NAMESPACE
