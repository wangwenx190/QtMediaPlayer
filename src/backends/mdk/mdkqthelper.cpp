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

#include "mdkqthelper.h"
#include "include/mdk/c/MediaInfo.h"
#include "include/mdk/c/Player.h"
#include "include/mdk/c/VideoFrame.h"
#include "include/mdk/c/global.h"
#include <QtCore/qdebug.h>
#include <QtCore/qlibrary.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>

#ifndef WWX190_GENERATE_MDKAPI
#define WWX190_GENERATE_MDKAPI(funcName, resultType, ...) \
    using _WWX190_MDKAPI_lp_##funcName = resultType(*)(__VA_ARGS__); \
    _WWX190_MDKAPI_lp_##funcName m_lp_##funcName = nullptr;
#endif

#ifndef WWX190_RESOLVE_MDKAPI
#define WWX190_RESOLVE_MDKAPI(funcName) \
    if (!m_lp_##funcName) { \
        qDebug() << "Resolving function:" << #funcName; \
        m_lp_##funcName = reinterpret_cast<_WWX190_MDKAPI_lp_##funcName>(library.resolve(#funcName)); \
        Q_ASSERT(m_lp_##funcName); \
        if (!m_lp_##funcName) { \
            qWarning() << "Failed to resolve function:" << #funcName; \
        } \
    }
#endif

#ifndef WWX190_CALL_MDKAPI
#define WWX190_CALL_MDKAPI(funcName, ...) \
    if (MDK::Qt::mdkData()->m_lp_##funcName) { \
        MDK::Qt::mdkData()->m_lp_##funcName(__VA_ARGS__); \
    }
#endif

#ifndef WWX190_CALL_MDKAPI_RETURN
#define WWX190_CALL_MDKAPI_RETURN(funcName, defVal, ...) \
    (MDK::Qt::mdkData()->m_lp_##funcName ? MDK::Qt::mdkData()->m_lp_##funcName(__VA_ARGS__) : defVal)
#endif

#ifndef WWX190_SETNULL_MDKAPI
#define WWX190_SETNULL_MDKAPI(funcName) \
    if (m_lp_##funcName) { \
        m_lp_##funcName = nullptr; \
    }
#endif

#ifndef WWX190_NOTNULL_MDKAPI
#define WWX190_NOTNULL_MDKAPI(funcName) (m_lp_##funcName != nullptr)
#endif

static const char _mdkHelper_mdk_fileName_envVar[] = "_WWX190_MDKPLAYER_MDK_FILENAME";

namespace MDK::Qt
{

struct MDKData
{
public:
    // global.h
    WWX190_GENERATE_MDKAPI(MDK_javaVM, void *, void *)
    WWX190_GENERATE_MDKAPI(MDK_setLogLevel, void, MDK_LogLevel)
    WWX190_GENERATE_MDKAPI(MDK_logLevel, MDK_LogLevel)
    WWX190_GENERATE_MDKAPI(MDK_setLogHandler, void, mdkLogHandler)
    WWX190_GENERATE_MDKAPI(MDK_setGlobalOptionString, void, const char *, const char *)
    WWX190_GENERATE_MDKAPI(MDK_setGlobalOptionInt32, void, const char *, int)
    WWX190_GENERATE_MDKAPI(MDK_setGlobalOptionPtr, void, const char *, void *)
    WWX190_GENERATE_MDKAPI(MDK_strdup, char *, const char *)
    WWX190_GENERATE_MDKAPI(MDK_version, int)

    // MediaInfo.h
    WWX190_GENERATE_MDKAPI(MDK_AudioStreamCodecParameters, void, const mdkAudioStreamInfo *, mdkAudioCodecParameters *)
    WWX190_GENERATE_MDKAPI(MDK_AudioStreamMetadata, bool, const mdkAudioStreamInfo *, mdkStringMapEntry *)
    WWX190_GENERATE_MDKAPI(MDK_VideoStreamCodecParameters, void, const mdkVideoStreamInfo *, mdkVideoCodecParameters *)
    WWX190_GENERATE_MDKAPI(MDK_VideoStreamMetadata, bool, const mdkVideoStreamInfo *, mdkStringMapEntry *)
    WWX190_GENERATE_MDKAPI(MDK_MediaMetadata, bool, const mdkMediaInfo *, mdkStringMapEntry *)

    // Player.h
    WWX190_GENERATE_MDKAPI(mdkPlayerAPI_new, const mdkPlayerAPI *)
    WWX190_GENERATE_MDKAPI(mdkPlayerAPI_delete, void, const struct mdkPlayerAPI **)
    WWX190_GENERATE_MDKAPI(MDK_foreignGLContextDestroyed, void)

    // VideoFrame.h
    WWX190_GENERATE_MDKAPI(mdkVideoFrameAPI_new, mdkVideoFrameAPI *, int, int, enum MDK_PixelFormat)
    WWX190_GENERATE_MDKAPI(mdkVideoFrameAPI_delete, void, struct mdkVideoFrameAPI **)

    explicit MDKData()
    {
        const bool result = load(qEnvironmentVariable(_mdkHelper_mdk_fileName_envVar, QStringLiteral("mdk")));
        Q_UNUSED(result);
    }

    ~MDKData()
    {
        const bool result = unload();
        Q_UNUSED(result);
    }

    [[nodiscard]] bool load(const QString &path)
    {
        Q_ASSERT(!path.isEmpty());
        if (path.isEmpty()) {
            qWarning() << "Failed to load MDK: empty library path.";
            return false;
        }

        if (isLoaded()) {
            qDebug() << "MDK already loaded. Unloading ...";
            if (!unload()) {
                return false;
            }
        }

        library.setFileName(path);
        const bool result = library.load();
        if (result) {
            // We can't get the full file name if QLibrary is not loaded.
            QFileInfo fi(library.fileName());
            fi.makeAbsolute();
            qDebug() << "Start loading MDK from:" << QDir::toNativeSeparators(fi.canonicalFilePath());
        } else {
            qDebug() << "Start loading MDK from:" << QDir::toNativeSeparators(path);
            qWarning() << "Failed to load MDK:" << library.errorString();
            return false;
        }

        // global.h
        WWX190_RESOLVE_MDKAPI(MDK_javaVM)
        WWX190_RESOLVE_MDKAPI(MDK_setLogLevel)
        WWX190_RESOLVE_MDKAPI(MDK_logLevel)
        WWX190_RESOLVE_MDKAPI(MDK_setLogHandler)
        WWX190_RESOLVE_MDKAPI(MDK_setGlobalOptionString)
        WWX190_RESOLVE_MDKAPI(MDK_setGlobalOptionInt32)
        WWX190_RESOLVE_MDKAPI(MDK_setGlobalOptionPtr)
        WWX190_RESOLVE_MDKAPI(MDK_strdup)
        WWX190_RESOLVE_MDKAPI(MDK_version)

        // MediaInfo.h
        WWX190_RESOLVE_MDKAPI(MDK_AudioStreamCodecParameters)
        WWX190_RESOLVE_MDKAPI(MDK_AudioStreamMetadata)
        WWX190_RESOLVE_MDKAPI(MDK_VideoStreamCodecParameters)
        WWX190_RESOLVE_MDKAPI(MDK_VideoStreamMetadata)
        WWX190_RESOLVE_MDKAPI(MDK_MediaMetadata)

        // Player.h
        WWX190_RESOLVE_MDKAPI(mdkPlayerAPI_new)
        WWX190_RESOLVE_MDKAPI(mdkPlayerAPI_delete)
        WWX190_RESOLVE_MDKAPI(MDK_foreignGLContextDestroyed)

        // VideoFrame.h
        WWX190_RESOLVE_MDKAPI(mdkVideoFrameAPI_new)
        WWX190_RESOLVE_MDKAPI(mdkVideoFrameAPI_delete)

        qDebug() << "MDK loaded successfully.";
        return true;
    }

    [[nodiscard]] bool unload()
    {
        // global.h
        WWX190_SETNULL_MDKAPI(MDK_javaVM)
        WWX190_SETNULL_MDKAPI(MDK_setLogLevel)
        WWX190_SETNULL_MDKAPI(MDK_logLevel)
        WWX190_SETNULL_MDKAPI(MDK_setLogHandler)
        WWX190_SETNULL_MDKAPI(MDK_setGlobalOptionString)
        WWX190_SETNULL_MDKAPI(MDK_setGlobalOptionInt32)
        WWX190_SETNULL_MDKAPI(MDK_setGlobalOptionPtr)
        WWX190_SETNULL_MDKAPI(MDK_strdup)
        WWX190_SETNULL_MDKAPI(MDK_version)

        // MediaInfo.h
        WWX190_SETNULL_MDKAPI(MDK_AudioStreamCodecParameters)
        WWX190_SETNULL_MDKAPI(MDK_AudioStreamMetadata)
        WWX190_SETNULL_MDKAPI(MDK_VideoStreamCodecParameters)
        WWX190_SETNULL_MDKAPI(MDK_VideoStreamMetadata)
        WWX190_SETNULL_MDKAPI(MDK_MediaMetadata)

        // Player.h
        WWX190_SETNULL_MDKAPI(mdkPlayerAPI_new)
        WWX190_SETNULL_MDKAPI(mdkPlayerAPI_delete)
        WWX190_SETNULL_MDKAPI(MDK_foreignGLContextDestroyed)

        // VideoFrame.h
        WWX190_SETNULL_MDKAPI(mdkVideoFrameAPI_new)
        WWX190_SETNULL_MDKAPI(mdkVideoFrameAPI_delete)

        if (library.isLoaded()) {
            if (!library.unload()) {
                qWarning() << "Failed to unload MDK:" << library.errorString();
                return false;
            }
        }

        qDebug() << "MDK unloaded successfully.";
        return true;
    }

    [[nodiscard]] bool isLoaded() const
    {
        const bool result =
                // global.h
                WWX190_NOTNULL_MDKAPI(MDK_javaVM) &&
                WWX190_NOTNULL_MDKAPI(MDK_setLogLevel) &&
                WWX190_NOTNULL_MDKAPI(MDK_logLevel) &&
                WWX190_NOTNULL_MDKAPI(MDK_setLogHandler) &&
                WWX190_NOTNULL_MDKAPI(MDK_setGlobalOptionString) &&
                WWX190_NOTNULL_MDKAPI(MDK_setGlobalOptionInt32) &&
                WWX190_NOTNULL_MDKAPI(MDK_setGlobalOptionPtr) &&
                WWX190_NOTNULL_MDKAPI(MDK_strdup) &&
                WWX190_NOTNULL_MDKAPI(MDK_version) &&
                // MediaInfo.h
                WWX190_NOTNULL_MDKAPI(MDK_AudioStreamCodecParameters) &&
                WWX190_NOTNULL_MDKAPI(MDK_AudioStreamMetadata) &&
                WWX190_NOTNULL_MDKAPI(MDK_VideoStreamCodecParameters) &&
                WWX190_NOTNULL_MDKAPI(MDK_VideoStreamMetadata) &&
                WWX190_NOTNULL_MDKAPI(MDK_MediaMetadata) &&
                // Player.h
                WWX190_NOTNULL_MDKAPI(mdkPlayerAPI_new) &&
                WWX190_NOTNULL_MDKAPI(mdkPlayerAPI_delete) &&
                WWX190_NOTNULL_MDKAPI(MDK_foreignGLContextDestroyed) &&
                // VideoFrame.h
                WWX190_NOTNULL_MDKAPI(mdkVideoFrameAPI_new) &&
                WWX190_NOTNULL_MDKAPI(mdkVideoFrameAPI_delete);
        return result;
    }

private:
    QLibrary library;
};

Q_GLOBAL_STATIC(MDKData, mdkData)

bool isMDKAvailable()
{
    return mdkData()->isLoaded();
}

QString getMDKVersion()
{
    const auto fullVerNum = MDK_version();
    const auto majorVerNum = (fullVerNum >> 16) & 0xff;
    const auto minorVerNum = (fullVerNum >> 8) & 0xff;
    const auto patchVerNum = fullVerNum & 0xff;
    return QStringLiteral("%1.%2.%3").arg(QString::number(majorVerNum),
                                          QString::number(minorVerNum),
                                          QString::number(patchVerNum));
}

} // namespace MDK::Qt

///////////////////////////////////////////
/// MDK
///////////////////////////////////////////

// global.h

void *MDK_javaVM(void *value)
{
    return WWX190_CALL_MDKAPI_RETURN(MDK_javaVM, nullptr, value);
}

void MDK_setLogLevel(MDK_LogLevel value)
{
    WWX190_CALL_MDKAPI(MDK_setLogLevel, value)
}

MDK_LogLevel MDK_logLevel()
{
    return WWX190_CALL_MDKAPI_RETURN(MDK_logLevel, MDK_LogLevel_Debug);
}

void MDK_setLogHandler(mdkLogHandler value)
{
    WWX190_CALL_MDKAPI(MDK_setLogHandler, value)
}

void MDK_setGlobalOptionString(const char *key, const char *value)
{
    WWX190_CALL_MDKAPI(MDK_setGlobalOptionString, key, value)
}

void MDK_setGlobalOptionInt32(const char *key, int value)
{
    WWX190_CALL_MDKAPI(MDK_setGlobalOptionInt32, key, value)
}

void MDK_setGlobalOptionPtr(const char *key, void *value)
{
    WWX190_CALL_MDKAPI(MDK_setGlobalOptionPtr, key, value)
}

char *MDK_strdup(const char *value)
{
    return WWX190_CALL_MDKAPI_RETURN(MDK_strdup, nullptr, value);
}

int MDK_version()
{
    return WWX190_CALL_MDKAPI_RETURN(MDK_version, MDK_VERSION);
}

// MediaInfo.h

void MDK_AudioStreamCodecParameters(const mdkAudioStreamInfo *asi, mdkAudioCodecParameters *acp)
{
    WWX190_CALL_MDKAPI(MDK_AudioStreamCodecParameters, asi, acp)
}

bool MDK_AudioStreamMetadata(const mdkAudioStreamInfo *asi, mdkStringMapEntry *sme)
{
    return WWX190_CALL_MDKAPI_RETURN(MDK_AudioStreamMetadata, false, asi, sme);
}

void MDK_VideoStreamCodecParameters(const mdkVideoStreamInfo *vsi, mdkVideoCodecParameters *vcp)
{
    WWX190_CALL_MDKAPI(MDK_VideoStreamCodecParameters, vsi, vcp)
}

bool MDK_VideoStreamMetadata(const mdkVideoStreamInfo *vsi, mdkStringMapEntry *sme)
{
    return WWX190_CALL_MDKAPI_RETURN(MDK_VideoStreamMetadata, false, vsi, sme);
}

bool MDK_MediaMetadata(const mdkMediaInfo *mi, mdkStringMapEntry *sme)
{
    return WWX190_CALL_MDKAPI_RETURN(MDK_MediaMetadata, false, mi, sme);
}

// Player.h

const mdkPlayerAPI *mdkPlayerAPI_new()
{
    return WWX190_CALL_MDKAPI_RETURN(mdkPlayerAPI_new, nullptr);
}

void mdkPlayerAPI_delete(const struct mdkPlayerAPI **value)
{
    WWX190_CALL_MDKAPI(mdkPlayerAPI_delete, value)
}

void MDK_foreignGLContextDestroyed()
{
    WWX190_CALL_MDKAPI(MDK_foreignGLContextDestroyed)
}

// VideoFrame.h

mdkVideoFrameAPI *mdkVideoFrameAPI_new(int w, int h, enum MDK_PixelFormat f)
{
    return WWX190_CALL_MDKAPI_RETURN(mdkVideoFrameAPI_new, nullptr, w, h, f);
}

void mdkVideoFrameAPI_delete(struct mdkVideoFrameAPI **value)
{
    WWX190_CALL_MDKAPI(mdkVideoFrameAPI_delete, value)
}
