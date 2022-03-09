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

#include <backendinterface.h>
#include "include/mdk/global.h"
#include "mdkplayer.h"
#include "mdkqthelper.h"
#include <QtQuick/qsgrendererinterface.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE
Q_LOGGING_CATEGORY(lcQMPMDK, "wangwenx190.qtmediaplayer.mdk")

static const QString kUnknown = QStringLiteral("Unknown");

[[nodiscard]] const QVariantHash &metaData_mdk()
{
    static const QVariantHash result = {
        {kName, QStringLiteral("MDK")},
        {kVersion, []() -> QString {
             if (!MDK::Qt::isMDKAvailable()) {
                 return kUnknown;
             }
             return MDK::Qt::getMDKVersion();
         }()},
        {kAuthors, QStringLiteral("Wang Bin <wbsecg1@gmail.com>")},
        {kCopyright, QStringLiteral("Copyright (C) 2019-2022 Wang Bin <wbsecg1@gmail.com>")},
        {kLicenses, QStringLiteral(R"(MIT License

Copyright (C) 2019-2022 Wang Bin <wbsecg1@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)")},
        {kHomepage, QStringLiteral("https://github.com/wang-bin/mdk-sdk/")},
        {kLastModifyTime, QString::fromUtf8(__DATE__ __TIME__)},
        {kLogo, {}}, // ### TODO
        {kFFmpegVersion, []() -> QString {
             if (!MDK::Qt::isMDKAvailable()) {
                 return kUnknown;
             }
             // MDK can only provide the major version of FFmpeg.
             int ver = 0;
             if (!MDK_NS_PREPEND(GetGlobalOption)("ffmpeg.version", &ver)) {
                 return kUnknown;
             }
             return QString::number(ver);
         }()},
        {kFFmpegConfiguration, []() -> QString {
             if (!MDK::Qt::isMDKAvailable()) {
                 return kUnknown;
             }
             const char *config = nullptr;
             if (!MDK_NS_PREPEND(GetGlobalOption)("ffmpeg.configuration", &config)) {
                 return kUnknown;
             }
             if (!config) {
                 return kUnknown;
             }
             return QString::fromUtf8(config);
         }()}
    };
    return result;
}

class MDKBackend final : public QMPBackend
{
    Q_DISABLE_COPY_MOVE(MDKBackend)

public:
    explicit MDKBackend() = default;
    ~MDKBackend() override = default;

    [[nodiscard]] QString name() const override
    {
        return metaData_mdk().value(kName).toString();
    }

    [[nodiscard]] QString version() const override
    {
        return metaData_mdk().value(kVersion).toString();
    }

    [[nodiscard]] bool available() const override
    {
        return MDK::Qt::isMDKAvailable();
    }

    [[nodiscard]] bool isGraphicsApiSupported(const int api) const override
    {
        switch (static_cast<QSGRendererInterface::GraphicsApi>(api)) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        case QSGRendererInterface::Direct3D11:
        case QSGRendererInterface::Vulkan:
        case QSGRendererInterface::Metal:
#endif
#if ((QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0)))
        case QSGRendererInterface::Direct3D11Rhi:
        case QSGRendererInterface::VulkanRhi:
        case QSGRendererInterface::MetalRhi:
        case QSGRendererInterface::OpenGLRhi:
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        case QSGRendererInterface::OpenGL:
#endif
            return true;
        default:
            return false;
        }
    }

    [[nodiscard]] QString filePath() const override
    {
        // ### TODO
        return {};
    }

    [[nodiscard]] QString fileName() const override
    {
        // ### TODO
        return {};
    }

    [[nodiscard]] bool initialize() const override
    {
        if (!available()) {
            return false;
        }
        if (m_initialized) {
            qCWarning(lcQMPMDK) << "This backend has already been initialized, don't initialize multiple times.";
            return true;
        }
        m_initialized = true;
        qRegisterMetaType<ChapterInfo>();
        qRegisterMetaType<Chapters>();
        qRegisterMetaType<MetaData>();
        qRegisterMetaType<MediaTracks>();
        qmlRegisterModule(QTMEDIAPLAYER_QML_URI, 1, 0);
        qmlRegisterUncreatableMetaObject(staticMetaObject, QTMEDIAPLAYER_QML_URI, 1, 0, "QtMediaPlayer",
              QStringLiteral("QtMediaPlayer is not creatable, it's only used for accessing enums & flags."));
        qmlRegisterType<MDKPlayer>(QTMEDIAPLAYER_QML_URI, 1, 0, "MediaPlayer");
        return true;
    }

private:
    static inline bool m_initialized = false;
};
QTMEDIAPLAYER_END_NAMESPACE

#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
extern "C" [[nodiscard]] bool QueryBackend_MDK
#else
extern "C" [[nodiscard]] QTMEDIAPLAYER_PLUGIN_API bool QueryBackend
#endif
(QTMEDIAPLAYER_PREPEND_NAMESPACE(QMPBackend) **ppBackend)
{
    Q_ASSERT(ppBackend);
    if (!ppBackend) {
        return false;
    }
    *ppBackend = new QTMEDIAPLAYER_PREPEND_NAMESPACE(MDKBackend);
    return true;
}
