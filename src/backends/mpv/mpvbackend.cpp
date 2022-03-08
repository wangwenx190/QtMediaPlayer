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

#include <clocale>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qscopeguard.h>
#include <QtQuick/qquickwindow.h>
#include <backendinterface.h>
#include "mpvplayer.h"
#include "mpvqthelper.h"

// Q_INIT_RESOURCE() can't be used inside namespace, we have to
// wrap it into a function outside of the namespace and use the
// wrapper function in the namespace instead.
static inline void initResource()
{
    Q_INIT_RESOURCE(mpvbackend);
}

QTMEDIAPLAYER_BEGIN_NAMESPACE
Q_LOGGING_CATEGORY(lcQMPMPV, "wangwenx190.qtmediaplayer.mpv")

static const QString kUnknown = QStringLiteral("Unknown");

[[nodiscard]] static inline QVariant mpvGetProperty(const QString &name)
{
    Q_ASSERT(!name.isEmpty());
    if (name.isEmpty()) {
        return {};
    }
    // If libmpv is not available, no need to continue executing.
    if (!MPV::Qt::isLibmpvAvailable()) {
        return {};
    }
    mpv_handle *mpv = mpv_create();
    Q_ASSERT(mpv);
    if (!mpv) {
        qCCritical(lcQMPMPV) << "Failed to create the mpv instance.";
        return {};
    }
    const auto cleanup = qScopeGuard([&mpv](){
        if (mpv) {
            mpv_terminate_destroy(mpv);
            mpv = nullptr;
        }
    });
    if (mpv_initialize(mpv) < 0) {
        qCCritical(lcQMPMPV) << "Failed to initialize the mpv player.";
        return {};
    }
    const QVariant result = MPV::Qt::get_property(mpv, name);
    const int errorCode = MPV::Qt::get_error(result);
    if (!result.isValid() || (errorCode < 0)) {
        qCWarning(lcQMPMPV) << "Failed to query property" << name << ':' << mpv_error_string(errorCode);
        return {};
    }
    return result;
}

[[nodiscard]] const QVariantHash &metaData_mpv()
{
    static const QVariantHash result = {
        {kName, QStringLiteral("MPV")},
        {kVersion, []() -> QString {
             if (!MPV::Qt::isLibmpvAvailable()) {
                 return kUnknown;
             }
             return MPV::Qt::getLibmpvVersion();
         }()},
        {kAuthors, QStringLiteral("The mpv developers")},
        {kCopyright, QStringLiteral("Copyright (C) The mpv developers")},
        {kLicenses, []() -> QString {
             initResource();
             const QDir dir(QStringLiteral(":/licenses"));
             if (!dir.exists()) {
                 return {};
             }
             const QFileInfoList fileInfoList = dir.entryInfoList(
                 QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
             if (fileInfoList.isEmpty()) {
                 return {};
             }
             QString text = {};
             for (auto&& fileInfo : qAsConst(fileInfoList)) {
                 QFile file(fileInfo.canonicalFilePath());
                 if (!file.open(QFile::ReadOnly | QFile::Text)) {
                     continue;
                 }
                 if (!text.isEmpty()) {
                     text.append(u'\n');
                 }
                 QTextStream stream(&file);
                 text.append(stream.readAll());
             }
             return text;
         }()},
        {kHomepage, QStringLiteral("https://mpv.io/")},
        {kLastModifyTime, QString::fromUtf8(__DATE__ __TIME__)},
        {kLogo, {}}, // ### TODO
        {kFFmpegVersion, []() -> QString {
             if (!MPV::Qt::isLibmpvAvailable()) {
                 return kUnknown;
             }
             const QString ver = mpvGetProperty(QStringLiteral("ffmpeg-version")).toString();
             return (ver.isEmpty() ? QStringLiteral("Unknown") : ver);
         }()},
        {kFFmpegConfiguration, []() -> QString {
             if (!MPV::Qt::isLibmpvAvailable()) {
                 return kUnknown;
             }
             // libmpv doesn't seem to provide the FFmpeg configuration parameters
             // , so just return mpv's own configuration parameters instead.
             const QString ver = mpvGetProperty(QStringLiteral("mpv-configuration")).toString();
             return (ver.isEmpty() ? QStringLiteral("Unknown") : ver);
         }()}
    };
    return result;
}

class MPVBackend final : public QMPBackend
{
    Q_DISABLE_COPY_MOVE(MPVBackend)

public:
    explicit MPVBackend() = default;
    ~MPVBackend() override = default;

    [[nodiscard]] QString name() const override
    {
        return metaData_mpv().value(kName).toString();
    }

    [[nodiscard]] QString version() const override
    {
        return metaData_mpv().value(kVersion).toString();
    }

    [[nodiscard]] bool available() const override
    {
        return MPV::Qt::isLibmpvAvailable();
    }

    [[nodiscard]] bool isGraphicsApiSupported(const int api) const override
    {
        switch (static_cast<QSGRendererInterface::GraphicsApi>(api)) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        case QSGRendererInterface::OpenGL:
#endif
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        case QSGRendererInterface::OpenGLRhi: // Equal to QSGRendererInterface::OpenGL in Qt6.
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
            qCWarning(lcQMPMPV) << "This backend has already been initialized, don't initialize multiple times.";
            return true;
        }
        m_initialized = true;
        // Qt sets the locale in the QGuiApplication constructor, but libmpv
        // requires the LC_NUMERIC category to be set to "C", so change it back.
        std::setlocale(LC_NUMERIC, "C");
        // Currently the mpv backend only supports rendering in OpenGL.
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#else
        // Although there is "QSGRendererInterface::OpenGL", we still prefer
        // the rhi enum value. They will go through different code paths inside
        // Qt Quick's scenegraph engine. But there will be no difference between
        // them since Qt6. It's the old Qt5 story.
        QQuickWindow::setSceneGraphBackend(QSGRendererInterface::OpenGLRhi);
#endif
        qRegisterMetaType<ChapterInfo>();
        qRegisterMetaType<Chapters>();
        qRegisterMetaType<MetaData>();
        qRegisterMetaType<MediaTracks>();
        qRegisterMetaType<MPV::Qt::ErrorReturn>();
        qmlRegisterModule(QTMEDIAPLAYER_QML_URI, 1, 0);
        qmlRegisterUncreatableMetaObject(staticMetaObject, QTMEDIAPLAYER_QML_URI, 1, 0, "QtMediaPlayer",
              QStringLiteral("QtMediaPlayer is not creatable, it's only used for accessing enums & flags."));
        qmlRegisterType<MPVPlayer>(QTMEDIAPLAYER_QML_URI, 1, 0, "MediaPlayer");
        return true;
    }

private:
    static inline bool m_initialized = false;
};
QTMEDIAPLAYER_END_NAMESPACE

#ifdef QTMEDIAPLAYER_PLUGIN_STATIC
[[nodiscard]] bool QueryBackend_MPV
#else
extern "C" [[nodiscard]] QTMEDIAPLAYER_PLUGIN_API bool QueryBackend
#endif
(QTMEDIAPLAYER_PREPEND_NAMESPACE(QMPBackend) **ppBackend)
{
    Q_ASSERT(ppBackend);
    if (!ppBackend) {
        return false;
    }
    *ppBackend = new QTMEDIAPLAYER_PREPEND_NAMESPACE(MPVBackend);
    return true;
}
