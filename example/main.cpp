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

#include <QtCore/qloggingcategory.h>
#include <QtCore/qcommandlineoption.h>
#include <QtCore/qcommandlineparser.h>
#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQuickControls2/qquickstyle.h>
#include <QtQuick/qquickwindow.h>
#include <qtmediaplayer.h>

int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif

    QCoreApplication::setApplicationName(QStringLiteral("QtMediaPlayer Demo"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0.0"));
    QCoreApplication::setOrganizationName(QStringLiteral("wangwenx190"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("wangwenx190.github.io"));

    QGuiApplication application(argc, argv);

    QCommandLineParser cmdLineParser;
    cmdLineParser.setApplicationDescription(QCoreApplication::translate("main", "QtMediaPlayer demo application."));
    cmdLineParser.addHelpOption();
    cmdLineParser.addVersionOption();
    cmdLineParser.addPositionalArgument(QStringLiteral("url"), QCoreApplication::translate("main", "URL to play."));

    const QCommandLineOption rhiBackendOption(QStringLiteral("rhi-backend"),
                                              QCoreApplication::translate("main", "Set the Qt RHI backend. Available backends: Direct3D, Vulkan, Metal, OpenGL, OpenGLES, Software, Auto."),
                                              QCoreApplication::translate("main", "backend"));
    cmdLineParser.addOption(rhiBackendOption);

    const QCommandLineOption playerBackendOption(QStringLiteral("player-backend"),
                                              QCoreApplication::translate("main", "Set the QtMediaPlayer backend. Available backends: MDK, MPV, VLC."),
                                              QCoreApplication::translate("main", "backend"));
    cmdLineParser.addOption(playerBackendOption);

    cmdLineParser.process(application);

    const QString rhiBackendParamValue = cmdLineParser.value(rhiBackendOption).toLower();
    const QString playerBackendParamValue = cmdLineParser.value(playerBackendOption).toLower();
    const QStringList positionalArguments = cmdLineParser.positionalArguments();

    // Same as setting the environment variable "QSG_INFO".
    QLoggingCategory::setFilterRules(QStringLiteral("qt.scenegraph.general=true\nqt.rhi.*=true"));

    if (!rhiBackendParamValue.isEmpty()) {
        // Same as setting the environment variable "QSG_RHI_BACKEND".
        if ((rhiBackendParamValue == QStringLiteral("direct3d"))
            || (rhiBackendParamValue == QStringLiteral("d3d"))) {
            qDebug() << "Setting Qt RHI backend to Direct3D ...";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11Rhi);
#else
            QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Direct3D11Rhi);
#endif
        } else if ((rhiBackendParamValue == QStringLiteral("vulkan"))
                   || (rhiBackendParamValue == QStringLiteral("vk"))) {
            qDebug() << "Setting Qt RHI backend to Vulkan ...";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QQuickWindow::setGraphicsApi(QSGRendererInterface::VulkanRhi);
#else
#endif
        } else if ((rhiBackendParamValue == QStringLiteral("opengl"))
                   || (rhiBackendParamValue == QStringLiteral("opengles"))
                   || (rhiBackendParamValue == QStringLiteral("gl"))
                   || (rhiBackendParamValue == QStringLiteral("gles"))) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
#else
            QQuickWindow::setSceneGraphBackend(QSGRendererInterface::OpenGLRhi);
#endif
            QSurfaceFormat surfaceFormat = QSurfaceFormat::defaultFormat();
            if (rhiBackendParamValue.contains(QStringLiteral("es"), Qt::CaseInsensitive)) {
                qDebug() << "Setting Qt RHI backend to OpenGL ES ...";
                // OpenGL ES, performance is better than desktop OpenGL.
                surfaceFormat.setRenderableType(QSurfaceFormat::OpenGLES);
                // The latest OpenGL ES version is V3.2 and will never be updated again in
                // the future because the next generation of OpenGL (ES) is Vulkan.
                surfaceFormat.setVersion(3, 2);
            } else {
                qDebug() << "Setting Qt RHI backend to OpenGL ...";
                // Desktop OpenGL
                surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
                // The latest OpenGL version is V4.6, released on October 22, 2019.
                surfaceFormat.setVersion(4, 6);
            }
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            QSurfaceFormat::setDefaultFormat(surfaceFormat);
        } else if ((rhiBackendParamValue == QStringLiteral("metal"))
                   || (rhiBackendParamValue == QStringLiteral("mt"))) {
            qDebug() << "Setting Qt RHI backend to Metal ...";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QQuickWindow::setGraphicsApi(QSGRendererInterface::MetalRhi);
#else
            QQuickWindow::setSceneGraphBackend(QSGRendererInterface::MetalRhi);
#endif
        } else if ((rhiBackendParamValue == QStringLiteral("software"))
                   || (rhiBackendParamValue == QStringLiteral("soft"))
                   || (rhiBackendParamValue == QStringLiteral("sw"))) {
            qDebug() << "Setting Qt RHI backend to Software ...";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
#else
            QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
#endif
        } else if (rhiBackendParamValue == QStringLiteral("auto")) {
            // Let Qt itself decide which RHI backend to use.
        } else {
            qWarning() << "Can't recognize the given RHI backend:" << rhiBackendParamValue;
            qDebug() << "Acceptable RHI backend names: Direct3D, Vulkan, Metal, OpenGL, OpenGLES, Software, Auto.";
            return -1;
        }
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QQuickStyle::setStyle(QStringLiteral("Basic"));
#else
    QQuickStyle::setStyle(QStringLiteral("Default"));
#endif

    QQmlApplicationEngine engine;

    if (playerBackendParamValue.isEmpty() || (playerBackendParamValue == QStringLiteral("mdk"))) {
        qDebug() << "Setting player backend to MDK ...";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        const QSGRendererInterface::GraphicsApi api = QQuickWindow::graphicsApi();
        if (!QTMEDIAPLAYER_PREPEND_NAMESPACE(isRHIBackendSupported)(QStringLiteral("MDK"), api)) {
            qWarning() << "Current Qt RHI backend" << api << "is not supported by the MDK backend.";
            return -1;
        }
#else
        // TODO
#endif
        if (!QTMEDIAPLAYER_PREPEND_NAMESPACE(initializeBackend)(QStringLiteral("MDK"))) {
            qWarning() << "Failed to create the MDK backend.";
            return -1;
        }
    } else if (playerBackendParamValue == QStringLiteral("mpv")) {
        qDebug() << "Setting player backend to MPV ...";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        const QSGRendererInterface::GraphicsApi api = QQuickWindow::graphicsApi();
        if (!QTMEDIAPLAYER_PREPEND_NAMESPACE(isRHIBackendSupported)(QStringLiteral("MPV"), api)) {
            qWarning() << "Current Qt RHI backend" << api << "is not supported by the MPV backend.";
            return -1;
        }
#else
        // TODO
#endif
        if (!QTMEDIAPLAYER_PREPEND_NAMESPACE(initializeBackend)(QStringLiteral("MPV"))) {
            qWarning() << "Failed to create the MPV backend.";
            return -1;
        }
    } else if (playerBackendParamValue == QStringLiteral("vlc")) {
        // TODO
    } else {
        qWarning() << "Can't recognize the given player backend:" << playerBackendParamValue;
        qDebug() << "Acceptable player backend names: MDK, MPV, VLC.";
        return -1;
    }

    const QUrl homePageURL(QStringLiteral("qrc:///qml/main.qml"));

    const QMetaObject::Connection connection = QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &application,
        [&homePageURL, &connection](QObject *object, const QUrl &url)
        {
            if (url != homePageURL) {
                return;
            }
            if (object) {
                QObject::disconnect(connection);
            } else {
                QGuiApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(homePageURL);

    return QGuiApplication::exec();
}
