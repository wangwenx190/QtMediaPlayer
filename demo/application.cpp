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

#include <QtCore/qdir.h>
#include <QtCore/qprocess.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qcommandlineoption.h>
#include <QtCore/qcommandlineparser.h>
#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQml/qqmlproperty.h>
#include <QtQuickControls2/qquickstyle.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qquickitem.h>
#include <qtmediaplayer.h>
#include <version.h>
#include "console.h"
#include "logger.h"
#include "constants.h"
#include "framelesswindow.h"
#include "history.h"
#include "i18n.h"
#include "os.h"
#include "settings.h"
#include "theme.h"
#include "application.h"
#ifdef Q_OS_WINDOWS
#  include "qtwinextras/qwintaskbarprogress.h"
#  include "qtwinextras/qquicktaskbarbutton_p.h"
#endif

static constexpr const char DEMO_APP_URI[] = "wangwenx190.QtMediaPlayer.Demo";
#ifdef Q_OS_WINDOWS
static constexpr const char QTWINEXTRAS_URI[] = "QtWinExtras";
#endif

static constexpr const char preferedBackendEnvVar[] = "QTMEDIAPLAYER_PREFERED_BACKEND";
static constexpr const char qtRhiDebugEnvVar[] = "QSG_INFO";
static constexpr const char qtRhiBackendEnvVar[] = "QSG_RHI_BACKEND";

static inline void dumpEnvironmentInfo()
{
    qInfo().noquote() << QCoreApplication::applicationName() << QCoreApplication::applicationVersion();
    qInfo() << "Built from commit" << QTMEDIAPLAYERDEMO_COMMIT_HASH << "on" << __TIME__ << ',' << __DATE__;
    qInfo() << "Application executable:" << QCoreApplication::applicationFilePath();
    qInfo() << "Working directory:" << QDir::currentPath();
    qInfo() << "Arguments:" << []() -> QStringList {
        QStringList arguments = QCoreApplication::arguments();
        // The first argument is always the full path to our executable,
        // the real command line arguments start from the second element.
        arguments.takeFirst();
        return arguments;
    }();
    qInfo() << "Environment variables:" << QProcessEnvironment::systemEnvironment().toStringList();
}

[[nodiscard]] static inline QSGRendererInterface::GraphicsApi stringToGraphicsApi(const QString &str)
{
    Q_ASSERT(!str.isEmpty());
    static const QSGRendererInterface::GraphicsApi defaultApi = []() -> QSGRendererInterface::GraphicsApi {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        return QSGRendererInterface::Null;
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        return QSGRendererInterface::NullRhi;
#else
        return QSGRendererInterface::Unknown;
#endif
    }();
    if (str.isEmpty()) {
        return defaultApi;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    if (str.contains(QStringLiteral("d3d"), Qt::CaseInsensitive)
        || str.contains(QStringLiteral("dx"), Qt::CaseInsensitive)
        || str.contains(QStringLiteral("win"), Qt::CaseInsensitive)) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        return QSGRendererInterface::Direct3D11;
#else
        return QSGRendererInterface::Direct3D11Rhi;
#endif
    }
    if (str.contains(QStringLiteral("vulkan"), Qt::CaseInsensitive)
        || str.contains(QStringLiteral("vk"), Qt::CaseInsensitive)
        || str.contains(QStringLiteral("lin"), Qt::CaseInsensitive)) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        return QSGRendererInterface::Vulkan;
#else
        return QSGRendererInterface::VulkanRhi;
#endif
    }
    if (str.contains(QStringLiteral("metal"), Qt::CaseInsensitive)
        || str.contains(QStringLiteral("mt"), Qt::CaseInsensitive)
        || str.contains(QStringLiteral("mac"), Qt::CaseInsensitive)) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        return QSGRendererInterface::Metal;
#else
        return QSGRendererInterface::MetalRhi;
#endif
    }
#endif
    if (str.contains(QStringLiteral("gl"), Qt::CaseInsensitive)) {
#if ((QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)) || (QT_VERSION < QT_VERSION_CHECK(5, 14, 0)))
        return QSGRendererInterface::OpenGL;
#else
        return QSGRendererInterface::OpenGLRhi;
#endif
    }
    if (str.contains(QStringLiteral("soft"), Qt::CaseInsensitive)
        || str.contains(QStringLiteral("sw"), Qt::CaseInsensitive)) {
        return QSGRendererInterface::Software;
    }
    return defaultApi;
}

int Application::run(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)) && (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

    QCoreApplication::setApplicationName(QStringLiteral("QtMediaPlayer Demo"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("QtMediaPlayer Demo"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0.0"));
    QCoreApplication::setOrganizationName(QStringLiteral("wangwenx190"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("wangwenx190.github.io"));

    QGuiApplication application(argc, argv);

    // Initialize the logger as soon as possible, but only after the construction
    // of QCoreApplication because we use some functions that require it exists.
    Logger::setup();

    QCommandLineParser cmdLineParser;
    cmdLineParser.setApplicationDescription(QCoreApplication::translate("main",
           "This simple demo application shows how to integrate QtMediaPlayer into Qt Quick applications."));
    cmdLineParser.addHelpOption();
    cmdLineParser.addVersionOption();
    cmdLineParser.addPositionalArgument(QStringLiteral("url"),
                                        QCoreApplication::translate("main", "The url to the media content you want to play."));

    const QCommandLineOption debugOption(QStringLiteral("debug"),
                                         QCoreApplication::translate("main", "Enable Qt RHI debug output."));
    cmdLineParser.addOption(debugOption);

    const QCommandLineOption consoleOption(QStringLiteral("console"),
                                           QCoreApplication::translate("main", "Create a console window to display the log messages."));
    cmdLineParser.addOption(consoleOption);

    const QCommandLineOption rhiOption(QStringLiteral("rhi"),
                                       QCoreApplication::translate("main", "Set Qt RHI backend to <backend>. Available options: d3d11/vulkan/metal/opengl/software."),
                                       QCoreApplication::translate("main", "backend"));
    cmdLineParser.addOption(rhiOption);

    const QCommandLineOption playerOption(QStringLiteral("player"),
                                          QCoreApplication::translate("main", "Set QtMediaPlayer player backend to <backend>. Available options: mdk/mpv/vlc/ffmpeg."),
                                          QCoreApplication::translate("main", "backend"));
    cmdLineParser.addOption(playerOption);

    cmdLineParser.process(application);

    const bool debugOptionValue = cmdLineParser.isSet(debugOption);
    const bool consoleOptionValue = cmdLineParser.isSet(consoleOption);
    const QString rhiOptionValue = cmdLineParser.value(rhiOption);
    const QString playerOptionValue = cmdLineParser.value(playerOption);
    const QStringList positionalArguments = cmdLineParser.positionalArguments();

    if (consoleOptionValue) {
        Console::create();
    }

    dumpEnvironmentInfo();

    // Don't overwrite env settings.
    if (!qEnvironmentVariableIsSet(qtRhiDebugEnvVar) && debugOptionValue) {
        // Same as setting the environment variable "QSG_INFO" to a non-zero value.
        QLoggingCategory::setFilterRules(QStringLiteral("qt.scenegraph.general=true\nqt.rhi.*=true"));
    }

    // Don't overwrite env settings.
    if (!qEnvironmentVariableIsSet(qtRhiBackendEnvVar) && !rhiOptionValue.isEmpty()) {
        const QSGRendererInterface::GraphicsApi api = stringToGraphicsApi(rhiOptionValue);
        if (api != QSGRendererInterface::Unknown) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            QQuickWindow::setGraphicsApi(api);
#else
            QQuickWindow::setSceneGraphBackend(api);
#endif
        }
    }

    // This is necessary for Qt6 because the default style has changed to platform native style.
    // Using the "qtquickcontrols2.conf" file can achieve the same effect.
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QQuickStyle::setStyle(QStringLiteral("Basic")); // The "Default" style has been renamed to "Basic".
#else
    QQuickStyle::setStyle(QStringLiteral("Default"));
#endif

    // Allocate all singleton types before the QML engine to ensure that they outlive it.
    // This is VERY important!
    QScopedPointer<Constants> constants(new Constants);
    QScopedPointer<History> history(new History);
    QScopedPointer<I18N> i18n(new I18N);
    QScopedPointer<OS> os(new OS);
    QScopedPointer<Settings> settings(new Settings);
    QScopedPointer<Theme> theme(new Theme);

    QQmlApplicationEngine engine;

    // Don't forget to do this!
    i18n->setEngine(&engine);

    // For this demo application, we add an extra search path to be convenient.
    QTMEDIAPLAYER_PREPEND_NAMESPACE(Loader::addPluginSearchPath)(
        QCoreApplication::applicationDirPath() + QStringLiteral("/qtmediaplayer"));
    const QStringList availableBackends = QTMEDIAPLAYER_PREPEND_NAMESPACE(Loader::getAvailableBackends)();
    if (availableBackends.isEmpty()) {
        qWarning() << "Can't find any available player backends for QtMediaPlayer.";
        return -1;
    }
    // Don't overwrite env settings.
    const QString preferedBackend = (qEnvironmentVariableIsSet(preferedBackendEnvVar)
                                         ? qEnvironmentVariable(preferedBackendEnvVar) : playerOptionValue);
    if (preferedBackend.isEmpty()) {
        bool inited = false;
        for (auto&& backend : qAsConst(availableBackends)) {
            Q_ASSERT(!backend.isEmpty());
            if (backend.isEmpty()) {
                continue;
            }
            if (QTMEDIAPLAYER_PREPEND_NAMESPACE(Loader::initializeBackend)(backend)) {
                inited = true;
                break;
            }
        }
        if (!inited) {
            qWarning() << "Failed to initialize any of the available player backends.";
            return -1;
        }
    } else {
        if (!availableBackends.contains(preferedBackend, Qt::CaseInsensitive)) {
            qWarning() << "The prefered player backend" << preferedBackend << "is not available.";
            return -1;
        }
        if (!QTMEDIAPLAYER_PREPEND_NAMESPACE(Loader::initializeBackend)(preferedBackend)) {
            qWarning() << "Failed to initialize the player backend" << preferedBackend;
            return -1;
        }
    }

#ifdef Q_OS_WINDOWS
    qmlRegisterModule(QTWINEXTRAS_URI, 1, 0);
    qmlRegisterUncreatableType<QWinTaskbarProgress>(QTWINEXTRAS_URI, 1, 0, "TaskbarProgress",
                    QStringLiteral("Cannot create TaskbarProgress - use TaskbarButton.progress instead."));
    qmlRegisterUncreatableType<QQuickTaskbarOverlay>(QTWINEXTRAS_URI, 1, 0, "TaskbarOverlay",
                      QStringLiteral("Cannot create TaskbarOverlay - use TaskbarButton.overlay instead."));
    qmlRegisterType<QQuickTaskbarButton>(QTWINEXTRAS_URI, 1, 0, "TaskbarButton");
#endif

    qmlRegisterModule(DEMO_APP_URI, 1, 0);
    qmlRegisterSingletonInstance(DEMO_APP_URI, 1, 0, "Constants", constants.data());
    qmlRegisterSingletonInstance(DEMO_APP_URI, 1, 0, "History", history.data());
    qmlRegisterSingletonInstance(DEMO_APP_URI, 1, 0, "I18N", i18n.data());
    qmlRegisterSingletonInstance(DEMO_APP_URI, 1, 0, "OS", os.data());
    qmlRegisterSingletonInstance(DEMO_APP_URI, 1, 0, "Settings", settings.data());
    qmlRegisterSingletonInstance(DEMO_APP_URI, 1, 0, "Theme", theme.data());
    qmlRegisterType<FramelessWindow>(DEMO_APP_URI, 1, 0, "FramelessWindow");
    qmlRegisterAnonymousType<QWindow, 254>(DEMO_APP_URI, 1);

    // The url is automatically generated by Qt's CMake script,
    // all dots in uri are replaced by slashes.
    const QUrl mainUrl(QStringLiteral("qrc:///wangwenx190/QtMediaPlayer/Demo/qml/MainWindow.qml"));

    const QMetaObject::Connection connection = QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &application,
        [&mainUrl, &connection](QObject *object, const QUrl &url)
        {
            if (url != mainUrl) {
                // The QML engine is creating some other components which we
                // are not interested in, so just ignore them.
                return;
            }
            if (object) {
                // Our main component has been created successfully,
                // this function is now useless, so disconnect.
                QObject::disconnect(connection);
            } else {
                // The QML engine failed to create our main component for
                // some reason, the engine itself will output some detailed debug
                // messages to the console for us, so we only need to exit the
                // application here.
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    // Start the real loading process.
    engine.load(mainUrl);

    const QObjectList rootObjects = engine.rootObjects();
    if (!positionalArguments.isEmpty() && !rootObjects.isEmpty()) {
        const auto window = qobject_cast<QQuickWindow *>(rootObjects.at(0));
        Q_ASSERT(window);
        if (!window) {
            qFatal("The main window is not created. This is very wrong.");
        }
        const auto player = window->findChild<QQuickItem *>(QStringLiteral("mediaPlayer"));
        Q_ASSERT(player);
        if (!player) {
            qFatal("The media player is not created. This is very wrong.");
        }
        for (auto &&arg : qAsConst(positionalArguments)) {
            Q_ASSERT(!arg.isEmpty());
            if (arg.isEmpty()) {
                continue;
            }
            const QUrl url = QUrl::fromUserInput(arg, QCoreApplication::applicationDirPath(),
                                                 QUrl::AssumeLocalFile);
            if (url.isValid()) {
                if (QQmlProperty::write(player, QStringLiteral("source"), url)) {
                    break;
                }
            }
        }
    }

    const int execResult = QCoreApplication::exec();

    qDebug() << "Exit from main event loop. Return code:" << execResult;

    if (Console::isPresent()) {
        Console::close();
    }

    return execResult;
}
