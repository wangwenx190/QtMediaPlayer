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
#include <backendinterface.h>
#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
#  include <mdkbackend.h>
#  include <mpvbackend.h>
//#  include <ffmpegbackend.h>
#else
#  include <QtCore/qdir.h>
#  include <QtCore/qfileinfo.h>
#  include <QtCore/qlibrary.h>
#endif

#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
#  ifndef QTMEDIAPLAYER_LOAD_STATIC_PLUGIN
#    define QTMEDIAPLAYER_LOAD_STATIC_PLUGIN(Name) \
       if (QueryBackend_##Name(&backend)) { \
           Q_CHECK_PTR(backend); \
           if (backend) { \
               const QString backendName = backend->name().toLower(); \
               if (backend->available()) { \
                   Q_ASSERT(!m_availableBackends.contains(backendName)); \
                   if (m_availableBackends.contains(backendName)) { \
                       qCWarning(lcQMPLoader) << "The player backend" << backendName << "is already recorded."; \
                   } else { \
                       m_availableBackends.insert(backendName, backend); \
                   } \
               } else { \
                   qCDebug(lcQMPLoader) << "The player backend" << backendName << "is not available."; \
                   backend->Release(); \
               } \
           } else { \
               qCWarning(lcQMPLoader) << "A null pointer is returned. This should not happen."; \
           } \
       } else { \
           qCWarning(lcQMPLoader) << "Failed to create backend instance. This should not happen."; \
       } \
       backend = nullptr;
#  endif
#endif

QTMEDIAPLAYER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQMPLoader, "wangwenx190.qtmediaplayer.loader")

#ifndef QTMEDIAPLAYER_PLUGIN_STATIC
static constexpr const char _qmp_backend_dir_envVar[] = "QTMEDIAPLAYER_BACKEND_SEARCH_PATH";
#endif

struct QMPData
{
    QMutex m_mutex = {};

#ifndef QTMEDIAPLAYER_PLUGIN_STATIC
    QStringList m_searchPaths = {};
#endif
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
#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
        QMutexLocker locker(&m_mutex);
        QMPBackend *backend = nullptr;
        QTMEDIAPLAYER_LOAD_STATIC_PLUGIN(MDK)
        QTMEDIAPLAYER_LOAD_STATIC_PLUGIN(MPV)
        //QTMEDIAPLAYER_LOAD_STATIC_PLUGIN(FFmpeg)
#else
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
#endif
    }

    inline void loadFromDir(const QString &path)
    {
#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
        Q_UNUSED(path);
#else
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
            const QString backendName = backend->name().toLower();
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
            m_availableBackends.insert(backendName, backend);
            m_mutex.unlock();
        }
#endif
    }

    inline void refreshCache()
    {
#ifndef QTMEDIAPLAYER_PLUGIN_STATIC
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
#endif
    }

    inline void addSearchDir(const QString &path)
    {
#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
        Q_UNUSED(path);
#else
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
#endif
    }

private:
    Q_DISABLE_COPY_MOVE(QMPData)
};

Q_GLOBAL_STATIC(QMPData, g_loaderHelper)

void Loader::addPluginSearchPath(const QString &value)
{
#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
    Q_UNUSED(value);
#else
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return;
    }
    g_loaderHelper()->addSearchDir(value);
#endif
}

QStringList Loader::getPluginSearchPaths()
{
#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
    return {};
#else
    QMutexLocker locker(&g_loaderHelper()->m_mutex);
    return g_loaderHelper()->m_searchPaths;
#endif
}

QStringList Loader::getAvailableBackends()
{
    QMutexLocker locker(&g_loaderHelper()->m_mutex);
    if (g_loaderHelper()->m_availableBackends.isEmpty()) {
        return {};
    }
    QStringList list = {};
    auto it = g_loaderHelper()->m_availableBackends.constBegin();
    while (it != g_loaderHelper()->m_availableBackends.constEnd()) {
        list << it.key();
        ++it;
    }
    return list;
}

bool Loader::initializeBackend(const QString &value)
{
    Q_ASSERT(!value.isEmpty());
    if (value.isEmpty()) {
        return false;
    }
    const QString loweredName = value.toLower();
    QMutexLocker locker(&g_loaderHelper()->m_mutex);
    if (!g_loaderHelper()->m_availableBackends.contains(loweredName)) {
        qCWarning(lcQMPLoader) << loweredName << "is not an available backend.";
        return false;
    }
    const auto backend = g_loaderHelper()->m_availableBackends.value(loweredName);
    Q_ASSERT(backend);
    if (!backend) {
        qCWarning(lcQMPLoader) << "A null pointer is returned, this is very wrong.";
        return false;
    }
    return backend->initialize();
}

bool Loader::isGraphicsApiSupported(const QString &name, const int api)
{
    Q_ASSERT(!name.isEmpty());
    if (name.isEmpty()) {
        return false;
    }
    const QString loweredName = name.toLower();
    QMutexLocker locker(&g_loaderHelper()->m_mutex);
    if (!g_loaderHelper()->m_availableBackends.contains(loweredName)) {
        qCWarning(lcQMPLoader) << loweredName << "is not an available backend.";
        return false;
    }
    const auto backend = g_loaderHelper()->m_availableBackends.value(loweredName);
    Q_ASSERT(backend);
    if (!backend) {
        qCWarning(lcQMPLoader) << "A null pointer is returned, this is very wrong.";
        return false;
    }
    return backend->isGraphicsApiSupported(api);
}

bool Loader::isLoaderStatic()
{
#ifdef QTMEDIAPLAYER_LOADER_STATIC
    return true;
#else
    return false;
#endif
}

bool Loader::isCommonStatic()
{
#ifdef QTMEDIAPLAYER_COMMON_STATIC
    return true;
#else
    return false;
#endif
}

bool Loader::isPluginStatic()
{
#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
    return true;
#else
    return false;
#endif
}

QTMEDIAPLAYER_END_NAMESPACE
