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

#include "framelesswindow.h"
#include <QtGui/qscreen.h>
#include <QtGui/qguiapplication.h>
#ifdef Q_OS_WINDOWS
#  include <QtQuickTemplates2/private/qquickbutton_p.h>
#  include <QtQuickTemplates2/private/qquickbutton_p_p.h>
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#    include <QtCore/qoperatingsystemversion.h>
#  else
#    include <QtCore/qsysinfo.h>
#  endif
#  include <QtCore/private/qsystemlibrary_p.h>
#  include <QtGui/qpa/qplatformwindow.h>
#  if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QtGui/qpa/qplatformnativeinterface.h>
#  else
#    include <QtGui/qpa/qplatformwindow_p.h>
#  endif
#  include <QtCore/qt_windows.h>
#  include <dwmapi.h>
#  include <shellapi.h>
#endif

#ifdef Q_OS_WINDOWS
Q_DECLARE_METATYPE(QMargins)

#ifndef WM_NCUAHDRAWCAPTION
#  define WM_NCUAHDRAWCAPTION (0x00AE)
#endif

#ifndef WM_NCUAHDRAWFRAME
#  define WM_NCUAHDRAWFRAME (0x00AF)
#endif

#ifndef SM_CXPADDEDBORDER
#  define SM_CXPADDEDBORDER (92)
#endif

#ifndef ABM_GETAUTOHIDEBAREX
#  define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
#  define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DPICHANGED
#  define WM_DPICHANGED (0x02E0)
#endif

#ifndef GET_X_LPARAM
#  define GET_X_LPARAM(lp) (static_cast<int>(static_cast<short>(LOWORD(lp))))
#endif

#ifndef GET_Y_LPARAM
#  define GET_Y_LPARAM(lp) (static_cast<int>(static_cast<short>(HIWORD(lp))))
#endif

#ifndef IsMinimized
#  define IsMinimized(hwnd) IsIconic(hwnd)
#endif

#ifndef IsMaximized
#  define IsMaximized(hwnd) IsZoomed(hwnd)
#endif

struct flh_timecaps_tag
{
    UINT wPeriodMin; // minimum period supported
    UINT wPeriodMax; // maximum period supported
};
using flh_TIMECAPS = flh_timecaps_tag;
using flh_PTIMECAPS = flh_timecaps_tag *;
using flh_NPTIMECAPS = flh_timecaps_tag * NEAR;
using flh_LPTIMECAPS = flh_timecaps_tag * FAR;
#endif

#ifdef Q_OS_WINDOWS
static const bool g_usePureQtImplementation =
    qEnvironmentVariableIntValue("QTMEDIAPLAYER_FRAMELESSWINDOW_PURE_QT") != 0;
#else
static constexpr const bool g_usePureQtImplementation = true;
#endif

static constexpr const int g_resizeBorderThickness = 8;
static constexpr const int g_titleBarHeight = 31;

#ifdef Q_OS_WINDOWS
static constexpr const int g_autoHideTaskbarThickness = 2; // The thickness of an auto-hide taskbar in pixels
#endif

[[nodiscard]] static inline QScreen *getCurrentScreen(const QQuickWindow * const window)
{
    Q_ASSERT(window);
    if (!window) {
        return nullptr;
    }
    QScreen *screen = window->screen();
    if (screen) {
        return screen;
    }
    return QGuiApplication::primaryScreen();
}

[[nodiscard]] static inline Qt::CursorShape calculateCursorShape
    (const QQuickWindow * const window, const QPointF &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return Qt::ArrowCursor;
    }
    if (window->visibility() != QQuickWindow::Windowed) {
        return Qt::ArrowCursor;
    }
    if (((pos.x() < g_resizeBorderThickness) && (pos.y() < g_resizeBorderThickness))
        || ((pos.x() >= (window->width() - g_resizeBorderThickness)) && (pos.y() >= (window->height() - g_resizeBorderThickness)))) {
        return Qt::SizeFDiagCursor;
    }
    if (((pos.x() >= (window->width() - g_resizeBorderThickness)) && (pos.y() < g_resizeBorderThickness))
        || ((pos.x() < g_resizeBorderThickness) && (pos.y() >= (window->height() - g_resizeBorderThickness)))) {
        return Qt::SizeBDiagCursor;
    }
    if ((pos.x() < g_resizeBorderThickness) || (pos.x() >= (window->width() - g_resizeBorderThickness))) {
        return Qt::SizeHorCursor;
    }
    if ((pos.y() < g_resizeBorderThickness) || (pos.y() >= (window->height() - g_resizeBorderThickness))) {
        return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
}

[[nodiscard]] static inline Qt::Edges calculateWindowEdges
    (const QQuickWindow * const window, const QPointF &pos)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
    if (window->visibility() != QQuickWindow::Windowed) {
        return {};
    }
    Qt::Edges edges = {};
    if (pos.x() < g_resizeBorderThickness) {
        edges |= Qt::LeftEdge;
    }
    if (pos.x() >= (window->width() - g_resizeBorderThickness)) {
        edges |= Qt::RightEdge;
    }
    if (pos.y() < g_resizeBorderThickness) {
        edges |= Qt::TopEdge;
    }
    if (pos.y() >= (window->height() - g_resizeBorderThickness)) {
        edges |= Qt::BottomEdge;
    }
    return edges;
}

#ifdef Q_OS_WINDOWS
[[nodiscard]] static inline bool IsWin8OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8);
#endif
    return result;
}

[[nodiscard]] static inline bool IsWin8Point1OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows8_1);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS8_1);
#endif
    return result;
}

[[nodiscard]] static inline bool IsDwmCompositionEnabled()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    // No need to do further test in this case.
    if (IsWin8OrGreater()) {
        return true;
    }
    static const auto pDwmIsCompositionEnabled =
        reinterpret_cast<decltype(&DwmIsCompositionEnabled)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmIsCompositionEnabled"));
    if (pDwmIsCompositionEnabled) {
        BOOL enabled = FALSE;
        const HRESULT hr = pDwmIsCompositionEnabled(&enabled);
        return (SUCCEEDED(hr) && (enabled != FALSE));
    }
    return false;
}

[[nodiscard]] static inline int GetResizeBorderThickness()
{
    // No need to call the "ForDpi" version, Windows will scale the value automatically
    // if we have set the DPI awareness level correctly in the manifest file.
    return GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
}

[[nodiscard]] static inline int GetTitleBarHeight()
{
    return GetSystemMetrics(SM_CYCAPTION) + GetResizeBorderThickness();
}

[[nodiscard]] static inline bool IsFullScreen(const HWND hwnd)
{
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return false;
    }
    RECT wndRect = {};
    GetWindowRect(hwnd, &wndRect);
    // According to Microsoft Docs, we should compare to primary screen's geometry.
    const HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    GetMonitorInfoW(mon, &mi);
    // Compare to the entire area of the screen, not the work area.
    const RECT scrRect = mi.rcMonitor;
    return ((wndRect.left == scrRect.left) && (wndRect.top == scrRect.top)
            && (wndRect.right == scrRect.right) && (wndRect.bottom == scrRect.bottom));
}

static inline void TriggerFrameChange(const HWND hwnd)
{
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return;
    }
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

static inline void FixupQtInternals(const HWND hwnd)
{
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return;
    }
    // Some correction is needed for the window class style and window style.
    // The original style doesn't have serious issues, but let's do the right thing.
    SetClassLongPtrW(hwnd, GCL_STYLE, GetClassLongPtrW(hwnd, GCL_STYLE) | CS_HREDRAW | CS_VREDRAW);
    SetWindowLongPtrW(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
    // According to Microsoft Docs, we should trigger a frame change event if we
    // changed the window style to make it take effect.
    TriggerFrameChange(hwnd);
}

static inline void UpdateWindowFrameMargins(const HWND hwnd)
{
    Q_ASSERT(hwnd);
    if (!hwnd) {
        return;
    }
    // We won't be able to extend the window frame if DWM composition is disabled.
    // No need to try further in this case.
    if (!IsDwmCompositionEnabled()) {
        return;
    }
    static const auto pDwmExtendFrameIntoClientArea =
        reinterpret_cast<decltype(&DwmExtendFrameIntoClientArea)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmExtendFrameIntoClientArea"));
    if (pDwmExtendFrameIntoClientArea) {
        // Our special handling of WM_NCCALCSIZE removed the whole window frame,
        // the frame shadow is lost at the same time because DWM won't draw any
        // frame shadow for frameless windows, we work-around this issue by extending
        // the window frame into the client area to pretend we still has some window
        // frame, the extended window frame won't be seen by the user anyway.
        // Since we only need to pretend that our window still has window frame and
        // the extended window frame is not visible in reality, extending one pixel
        // will be enough, though any positive number will also work.
        const MARGINS dwmMargins = {1, 1, 1, 1};
        pDwmExtendFrameIntoClientArea(hwnd, &dwmMargins);
        // Force a WM_NCCALCSIZE event to make our new window frame take effect.
        TriggerFrameChange(hwnd);
    }
}

static inline void UpdateQtInternalFrame(QQuickWindow *window)
{
    Q_ASSERT(window);
    if (!window) {
        return;
    }
    // The following code is to inform Qt about our custom window frame size,
    // otherwise Qt will always consider the standard frame size whenever we
    // change the window geometry programatically, and that will cause wrong
    // offset everytime the window geometry changes, it will not be good.
    const int frameWidth = GetResizeBorderThickness();
    const QMargins qtMargins = {-frameWidth, -GetTitleBarHeight(), -frameWidth, -frameWidth};
    const QVariant qtMarginsVar = QVariant::fromValue(qtMargins);
    window->setProperty("_q_windowsCustomMargins", qtMarginsVar);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (QPlatformWindow *platformWindow = window->handle()) {
        QGuiApplication::platformNativeInterface()->setWindowProperty(platformWindow, QStringLiteral("WindowsCustomMargins"), qtMarginsVar);
    }
#else
    if (const auto platformWindow = dynamic_cast<QNativeInterface::Private::QWindowsWindow *>(window->handle())) {
        platformWindow->setCustomMargins(qtMargins);
    }
#endif
}

static inline void SyncWmPaintWithDwm()
{
    // No need to sync with DWM if DWM composition is disabled.
    if (!IsDwmCompositionEnabled()) {
        return;
    }
    QSystemLibrary winmmLib(QStringLiteral("winmm"));
    static const auto ptimeGetDevCaps =
        reinterpret_cast</*MMRESULT*/UINT(WINAPI *)(flh_LPTIMECAPS, UINT)>(winmmLib.resolve("timeGetDevCaps"));
    static const auto ptimeBeginPeriod =
        reinterpret_cast</*MMRESULT*/UINT(WINAPI *)(UINT)>(winmmLib.resolve("timeBeginPeriod"));
    static const auto ptimeEndPeriod =
        reinterpret_cast</*MMRESULT*/UINT(WINAPI *)(UINT)>(winmmLib.resolve("timeEndPeriod"));
    static const auto pDwmGetCompositionTimingInfo =
        reinterpret_cast<HRESULT(WINAPI *)(HWND, DWM_TIMING_INFO *)>(QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmGetCompositionTimingInfo"));
    if (!ptimeGetDevCaps || !ptimeBeginPeriod || !ptimeEndPeriod || !pDwmGetCompositionTimingInfo) {
        return;
    }
    // Dirty hack to workaround the resize flicker caused by DWM.
    LARGE_INTEGER freq = {};
    QueryPerformanceFrequency(&freq);
    flh_TIMECAPS tc = {};
    ptimeGetDevCaps(&tc, sizeof(tc));
    const UINT ms_granularity = tc.wPeriodMin;
    ptimeBeginPeriod(ms_granularity);
    LARGE_INTEGER now0 = {};
    QueryPerformanceCounter(&now0);
    // ask DWM where the vertical blank falls
    DWM_TIMING_INFO dti;
    SecureZeroMemory(&dti, sizeof(dti));
    dti.cbSize = sizeof(dti);
    pDwmGetCompositionTimingInfo(nullptr, &dti);
    LARGE_INTEGER now1 = {};
    QueryPerformanceCounter(&now1);
    // - DWM told us about SOME vertical blank
    //   - past or future, possibly many frames away
    // - convert that into the NEXT vertical blank
    const LONGLONG period = dti.qpcRefreshPeriod;
    const LONGLONG dt = dti.qpcVBlank - now1.QuadPart;
    LONGLONG w = 0, m = 0;
    if (dt >= 0) {
        w = dt / period;
    } else {
        // reach back to previous period
        // - so m represents consistent position within phase
        w = -1 + dt / period;
    }
    m = dt - (period * w);
    Q_ASSERT(m >= 0);
    Q_ASSERT(m < period);
    const qreal m_ms = 1000.0 * static_cast<qreal>(m) / static_cast<qreal>(freq.QuadPart);
    Sleep(static_cast<DWORD>(qRound(m_ms)));
    ptimeEndPeriod(ms_granularity);
}

enum class MouseEventType : int
{
    NotInterested = -1,
    Hover = 0,
    Down = 1,
    Up = 2
};

enum class MousePosType : int
{
    Unknown = -1,
    Global = 0,
    Local = 1,
    Screen = Global,
    Window = Local,
    Scene = Local,
    Client = Local
};

struct MousePos
{
    QPointF pos = {};
    MousePosType type = MousePosType::Unknown;
};

struct SystemButtonEvent
{
    const QQuickWindow *window = nullptr;
    MouseEventType type = MouseEventType::NotInterested;
    MousePos mousePos = {};
};

[[nodiscard]] static inline std::optional<LRESULT>
    handleSystemButtonEvent(const SystemButtonEvent * const event)
{
    Q_ASSERT(event);
    Q_ASSERT(event->window);
    Q_ASSERT(event->type != MouseEventType::NotInterested);
    Q_ASSERT(event->mousePos.type != MousePosType::Unknown);
    if (!event || !event->window
        || (event->type == MouseEventType::NotInterested)
        || (event->mousePos.type == MousePosType::Unknown)) {
        return std::nullopt;
    }
    static const auto minBtn = event->window->findChild<QQuickButton *>(QStringLiteral("TitleBar_SystemButton_Minimize"));
    static const auto maxBtn = event->window->findChild<QQuickButton *>(QStringLiteral("TitleBar_SystemButton_Maximize"));
    static const auto closeBtn = event->window->findChild<QQuickButton *>(QStringLiteral("TitleBar_SystemButton_Close"));
    if (!minBtn && !maxBtn && !closeBtn) {
        return std::nullopt;
    }
    const auto isInsideButton = [](const QQuickButton * const button, const QPointF &mousePos) -> bool {
        Q_ASSERT(button);
        if (!button) {
            return false;
        }
        const QPointF topLeft = button->mapToScene(QPointF(0.0, 0.0));
        const QSizeF size = button->size();
        return QRectF(topLeft, size).contains(mousePos);
    };
    const QPointF windowPos = [event]() -> QPointF {
        const qreal dpr = event->window->effectiveDevicePixelRatio();
        if (event->mousePos.type == MousePosType::Global) {
            return event->window->mapFromGlobal(event->mousePos.pos);
        } else if (event->mousePos.type == MousePosType::Local) {
            return QPointF(event->mousePos.pos / dpr);
        } else {
            Q_ASSERT(false);
            return {};
        }
    }();
    const bool insideMin = minBtn && isInsideButton(minBtn, windowPos);
    const bool insideMax = maxBtn && isInsideButton(maxBtn, windowPos);
    const bool insideClose = closeBtn && isInsideButton(closeBtn, windowPos);
    const auto resetAllButtons = [](){
        if (minBtn) {
            minBtn->resetDown();
            minBtn->setHovered(false);
        }
        if (maxBtn) {
            maxBtn->resetDown();
            maxBtn->setHovered(false);
        }
        if (closeBtn) {
            closeBtn->resetDown();
            closeBtn->setHovered(false);
        }
    };
    switch (event->type) {
    case MouseEventType::NotInterested:
        Q_ASSERT(false);
        break;
    case MouseEventType::Hover: {
        qDebug() << "<<<<<<<<<<<<<";
        resetAllButtons();
        if (insideMin) {
            minBtn->setHovered(true);
            return HTMINBUTTON;
        } else if (insideMax) {
            maxBtn->setHovered(true);
            return HTMAXBUTTON;
        } else if (insideClose) {
            closeBtn->setHovered(true);
            return HTCLOSE;
        }
    } break;
    case MouseEventType::Down: {
        qDebug() << "---------------------";
        resetAllButtons();
        if (insideMin) {
            minBtn->setDown(true);
        } else if (insideMax) {
            maxBtn->setDown(true);
        } else if (insideClose) {
            closeBtn->setDown(true);
        }
    } break;
    case MouseEventType::Up: {
        qDebug() << "!!!!!!!!!!!";
        resetAllButtons();
        if (insideMin) {
            qDebug() << "AAAAAAA";
            minBtn->setHovered(true);
            QQuickButtonPrivate::get(minBtn)->click();
        } else if (insideMax) {
            qDebug() << "BBBBBB";
            maxBtn->setHovered(true);
            QQuickButtonPrivate::get(maxBtn)->click();
        } else if (insideClose) {
            qDebug() << "CCCCCCC";
            closeBtn->setHovered(true);
            QQuickButtonPrivate::get(closeBtn)->click();
        }
    } break;
    }
    return std::nullopt;
}
#endif

FramelessWindow::FramelessWindow(QWindow *parent) : QQuickWindow(parent)
{
    initialize();
}

FramelessWindow::~FramelessWindow() = default;

bool FramelessWindow::isHidden() const
{
    return (visibility() == Hidden);
}

bool FramelessWindow::isMinimized() const
{
    return (visibility() == Minimized);
}

bool FramelessWindow::isMaximized() const
{
    return (visibility() == Maximized);
}

bool FramelessWindow::isFullScreen() const
{
    return (visibility() == FullScreen);
}

void FramelessWindow::showMinimized2()
{
#ifdef Q_OS_WINDOWS
    const auto hwnd = reinterpret_cast<HWND>(winId());
    Q_ASSERT(hwnd);
    if (hwnd) {
        // Work-around a QtQuick bug: https://bugreports.qt.io/browse/QTBUG-69711
        // Don't use "SW_SHOWMINIMIZED" because it will activate the current window
        // instead of the next window in the Z order, which is not the default behavior
        // of native Win32 applications.
        ShowWindow(hwnd, SW_MINIMIZE);
    } else {
        showMinimized();
    }
#else
    showMinimized();
#endif
}

void FramelessWindow::toggleMaximized()
{
    if (isHidden() || isMinimized()) {
        return;
    }
    if (isMaximized()) {
        setVisibility(m_savedVisibility);
    } else {
        m_savedVisibility = visibility();
        showMaximized();
    }
}

void FramelessWindow::toggleFullScreen()
{
    if (isHidden() || isMinimized()) {
        return;
    }
    if (isFullScreen()) {
        setVisibility(m_savedVisibility);
    } else {
        m_savedVisibility = visibility();
        showFullScreen();
    }
}

void FramelessWindow::bringToFront()
{
    if (isHidden()) {
        setVisible(true);
    }
    if (isMinimized()) {
        setWindowStates(windowStates() & ~Qt::WindowMinimized);
    }
    raise();
    requestActivate();
}

void FramelessWindow::moveToCenter()
{
    const QScreen * const _screen = getCurrentScreen(this);
    Q_ASSERT(_screen);
    if (!_screen) {
        return;
    }
    // Use QScreen::availableSize() to take the taskbar into account.
    const QSize screenSize = _screen->availableSize();
    const int newX = qRound(qreal(screenSize.width() - width()) / 2.0);
    const int newY = qRound(qreal(screenSize.height() - height()) / 2.0);
    // This offset is needed when the user put their taskbar on the top or left side.
    const QPoint offset = _screen->availableGeometry().topLeft();
    setX(newX + offset.x());
    setY(newY + offset.y());
}

bool FramelessWindow::startSystemResize2(const Qt::Edges edges)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    return startSystemResize(edges);
#else
    qWarning() << "QWindow::startSystemResize() is only available since Qt 5.15.0";
    return false;
#endif
}

bool FramelessWindow::startSystemMove2()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    return startSystemMove();
#else
    qWarning() << "QWindow::startSystemMove() is only available since Qt 5.15.0";
    return false;
#endif
}

void FramelessWindow::zoomIn(const qreal step)
{
    const QScreen * const _screen = getCurrentScreen(this);
    Q_ASSERT(_screen);
    if (!_screen) {
        return;
    }
    const int oldX = x();
    const int oldY = y();
    const int oldWidth = width();
    const int oldHeight = height();
    const qreal factor = qBound(qreal(0.001), step, qreal(1.0));
    const auto widthDiff = static_cast<int>(qRound(qreal(_screen->size().width()) * factor));
    const int newWidth = oldWidth + widthDiff;
    const auto newHeight = static_cast<int>(qRound(qreal(oldHeight) * qreal(newWidth) / qreal(oldWidth)));
    const int heightDiff = newHeight - oldHeight;
    const int newX = oldX - static_cast<int>(qRound(qreal(widthDiff) / 2.0));
    const int newY = oldY - static_cast<int>(qRound(qreal(heightDiff) / 2.0));
    setX(newX);
    setY(newY);
    setWidth(newWidth);
    setHeight(newHeight);
}

void FramelessWindow::zoomOut(const qreal step)
{
    const QScreen * const _screen = getCurrentScreen(this);
    Q_ASSERT(_screen);
    if (!_screen) {
        return;
    }
    const int oldX = x();
    const int oldY = y();
    const int oldWidth = width();
    const int oldHeight = height();
    const qreal factor = qBound(qreal(0.001), step, qreal(1.0));
    const auto widthDiff = static_cast<int>(qRound(qreal(_screen->size().width()) * factor));
    const int newWidth = oldWidth - widthDiff;
    const auto newHeight = static_cast<int>(qRound(qreal(oldHeight) * qreal(newWidth) / qreal(oldWidth)));
    const int heightDiff = oldHeight - newHeight;
    const int newX = oldX + static_cast<int>(qRound(qreal(widthDiff) / 2.0));
    const int newY = oldY + static_cast<int>(qRound(qreal(heightDiff) / 2.0));
    setX(newX);
    setY(newY);
    setWidth(newWidth);
    setHeight(newHeight);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool FramelessWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool FramelessWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
    const bool filtered = QQuickWindow::nativeEvent(eventType, message, result);
    if (filtered) {
        return true;
    }
#ifdef Q_OS_WINDOWS
    // We deliberately only use native Win32 APIs instead of the Qt APIs inside this
    // function because Qt APIs will do some magic work to the numbers to let them
    // work cross platforms and thus they will be inconsistent with the values returned
    // by the Win32 APIs, that will cause problems. Simply avoid using Qt APIs inside
    // this function will work-around this issue.
    if (g_usePureQtImplementation) {
        return false;
    }
    // I don't think this check is necessary, but since the Qt documentation has
    // this in the example code, we also add this just to be extra safe.
    if ((eventType != QByteArrayLiteral("windows_generic_MSG")) || !message || !result) {
        return false;
    }
    const auto msg = static_cast<LPMSG>(message);
    if (!msg->hwnd) {
        // The "hwnd" member sometimes will be null, don't know why.
        // We need to skip the current processing in such situations.
        return false;
    }
    switch (msg->message) {
    case WM_NCCALCSIZE: {
        // If `wParam` is `FALSE`, `lParam` points to a `RECT` that contains
        // the proposed window rectangle for our window.  During our
        // processing of the `WM_NCCALCSIZE` message, we are expected to
        // modify the `RECT` that `lParam` points to, so that its value upon
        // our return is the new client area.  We must return 0 if `wParam`
        // is `FALSE`.
        //
        // If `wParam` is `TRUE`, `lParam` points to a `NCCALCSIZE_PARAMS`
        // struct.  This struct contains an array of 3 `RECT`s, the first of
        // which has the exact same meaning as the `RECT` that is pointed to
        // by `lParam` when `wParam` is `FALSE`.  The remaining `RECT`s, in
        // conjunction with our return value, can
        // be used to specify portions of the source and destination window
        // rectangles that are valid and should be preserved.  We opt not to
        // implement an elaborate client-area preservation technique, and
        // simply return 0, which means "preserve the entire old client area
        // and align it with the upper-left corner of our new client area".
        const auto clientRect = (msg->wParam
                                     ? &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(msg->lParam))->rgrc[0]
                                     : reinterpret_cast<LPRECT>(msg->lParam));
        const bool max = IsMaximized(msg->hwnd);
        const bool full = IsFullScreen(msg->hwnd);
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        if (max && !full) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            const int frameWidth = GetResizeBorderThickness();
            clientRect->top += frameWidth;
            clientRect->bottom -= frameWidth;
            clientRect->left += frameWidth;
            clientRect->right -= frameWidth;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (max || full) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            const UINT taskbarState = SHAppBarMessage(ABM_GETSTATE, &abd);
            // First, check if we have an auto-hide taskbar at all:
            if (taskbarState & ABS_AUTOHIDE) {
                bool top = false, bottom = false, left = false, right = false;
                // Due to ABM_GETAUTOHIDEBAREX only exists from Win8.1,
                // we have to use another way to judge this if we are
                // running on Windows 7 or Windows 8.
                if (IsWin8Point1OrGreater()) {
                    MONITORINFO monitorInfo;
                    SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
                    monitorInfo.cbSize = sizeof(monitorInfo);
                    const HMONITOR monitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
                    GetMonitorInfoW(monitor, &monitorInfo);
                    // This helper can be used to determine if there's a
                    // auto-hide taskbar on the given edge of the monitor
                    // we're currently on.
                    const auto hasAutohideTaskbar = [&monitorInfo](const UINT edge) -> bool {
                        APPBARDATA _abd;
                        SecureZeroMemory(&_abd, sizeof(_abd));
                        _abd.cbSize = sizeof(_abd);
                        _abd.uEdge = edge;
                        _abd.rc = monitorInfo.rcMonitor;
                        const auto hTaskbar = reinterpret_cast<HWND>(SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &_abd));
                        return (hTaskbar != nullptr);
                    };
                    top = hasAutohideTaskbar(ABE_TOP);
                    bottom = hasAutohideTaskbar(ABE_BOTTOM);
                    left = hasAutohideTaskbar(ABE_LEFT);
                    right = hasAutohideTaskbar(ABE_RIGHT);
                } else {
                    int edge = -1;
                    APPBARDATA _abd;
                    SecureZeroMemory(&_abd, sizeof(_abd));
                    _abd.cbSize = sizeof(_abd);
                    _abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
                    const HMONITOR windowMonitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
                    const HMONITOR taskbarMonitor = MonitorFromWindow(_abd.hWnd, MONITOR_DEFAULTTOPRIMARY);
                    if (taskbarMonitor == windowMonitor) {
                        SHAppBarMessage(ABM_GETTASKBARPOS, &_abd);
                        edge = _abd.uEdge;
                    }
                    top = (edge == ABE_TOP);
                    bottom = (edge == ABE_BOTTOM);
                    left = (edge == ABE_LEFT);
                    right = (edge == ABE_RIGHT);
                }
                // If there's a taskbar on any side of the monitor, reduce
                // our size a little bit on that edge.
                // Note to future code archeologists:
                // This doesn't seem to work for fullscreen on the primary
                // display. However, testing a bunch of other apps with
                // fullscreen modes and an auto-hiding taskbar has
                // shown that _none_ of them reveal the taskbar from
                // fullscreen mode. This includes Edge, Firefox, Chrome,
                // Sublime Text, PowerPoint - none seemed to support this.
                // This does however work fine for maximized.
                if (top) {
                    // Peculiarly, when we're fullscreen,
                    clientRect->top += g_autoHideTaskbarThickness;
                } else if (bottom) {
                    clientRect->bottom -= g_autoHideTaskbarThickness;
                } else if (left) {
                    clientRect->left += g_autoHideTaskbarThickness;
                } else if (right) {
                    clientRect->right -= g_autoHideTaskbarThickness;
                }
            }
        }
        SyncWmPaintWithDwm();
        // We want to reduce flicker during resize by returning "WVR_REDRAW"
        // but Windows will exhibits bugs where lient pixels and child windows
        // are mispositioned by the width/height of the upper-left nonclient area.
        // But since we know we won't have any child windows, we are safe to do this.
        // But we must return zero when wParam is FALSE, according to Microsoft Docs.
        *result = (msg->wParam ? WVR_REDRAW : 0);
        return true;
    }
    case WM_NCHITTEST: {
        RECT rect = {};
        GetWindowRect(msg->hwnd, &rect);
        const int width = qAbs(rect.right - rect.left);
        const int height = qAbs(rect.bottom - rect.top);
        const POINT globalPos = {GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
        POINT localPos = globalPos;
        ScreenToClient(msg->hwnd, &localPos);
        const int frameSize = GetResizeBorderThickness();
        const bool isTop = (localPos.y < frameSize);
        const bool isBottom = (localPos.y >= (height - frameSize));
        const bool isLeft = (localPos.x < frameSize);
        const bool isRight = (localPos.x >= (width - frameSize));
        *result = [this, &msg, isTop, isBottom, isLeft, isRight]() -> LRESULT {
            const auto xPos = GET_X_LPARAM(msg->lParam);
            const auto yPos = GET_Y_LPARAM(msg->lParam);
            SystemButtonEvent event = {};
            event.window = this;
            event.type = MouseEventType::Hover;
            event.mousePos.type = MousePosType::Screen;
            event.mousePos.pos = QPointF(qreal(xPos), qreal(yPos));
            const auto sysBtnHitTestResult = handleSystemButtonEvent(&event);
            if (sysBtnHitTestResult.has_value()) {
                return sysBtnHitTestResult.value();
            }
            if (IsMaximized(msg->hwnd) || IsFullScreen(msg->hwnd)) {
                return HTCLIENT;
            }
            if (isTop) {
                if (isLeft) {
                    return HTTOPLEFT;
                }
                if (isRight) {
                    return HTTOPRIGHT;
                }
                return HTTOP;
            }
            if (isBottom) {
                if (isLeft) {
                    return HTBOTTOMLEFT;
                }
                if (isRight) {
                    return HTBOTTOMRIGHT;
                }
                return HTBOTTOM;
            }
            if (isLeft) {
                return HTLEFT;
            }
            if (isRight) {
                return HTRIGHT;
            }
            return HTCLIENT;
        }();
        return true;
    }
    case WM_WINDOWPOSCHANGING: {
        // Tell Windows to discard the entire contents of the client area, as re-using
        // parts of the client area would lead to jitter during resize.
        const auto windowPos = reinterpret_cast<LPWINDOWPOS>(msg->lParam);
        windowPos->flags |= SWP_NOCOPYBITS;
    } break;
    case WM_DWMCOMPOSITIONCHANGED: {
        UpdateWindowFrameMargins(msg->hwnd);
    } break;
    case WM_DPICHANGED: {
        // Sync the internal frame margins with the latest DPI settings.
        UpdateQtInternalFrame(this);
        TriggerFrameChange(msg->hwnd);
    } break;
    case WM_NCUAHDRAWCAPTION:
    case WM_NCUAHDRAWFRAME: {
        // These undocumented messages are sent to draw themed window
        // borders. Block them to prevent drawing borders over the client
        // area.
        *result = 0;
        return true;
    }
    case WM_NCPAINT: {
        if (!IsDwmCompositionEnabled()) {
            // Only block WM_NCPAINT when DWM composition is disabled. If
            // it's blocked when DWM composition is enabled, the frame
            // shadow won't be drawn.
            *result = 0;
            return true;
        }
    } break;
    case WM_NCACTIVATE: {
        if (IsDwmCompositionEnabled()) {
            // DefWindowProc won't repaint the window border if lParam
            // (normally a HRGN) is -1. See the following link's "lParam"
            // section:
            // https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-ncactivate
            // Don't use "*result = 0" otherwise the window won't respond
            // to the window active state change.
            *result = DefWindowProcW(msg->hwnd, WM_NCACTIVATE, msg->wParam, -1);
        } else {
            *result = (msg->wParam ? FALSE : TRUE);
        }
        return true;
    }
    case WM_SETICON:
    case WM_SETTEXT: {
        // Disable painting while these messages are handled to prevent them
        // from drawing a window caption over the client area.
        const LONG_PTR oldStyle = GetWindowLongPtrW(msg->hwnd, GWL_STYLE);
        // Prevent Windows from drawing the default title bar by temporarily
        // toggling the WS_VISIBLE style.
        SetWindowLongPtrW(msg->hwnd, GWL_STYLE, static_cast<LONG_PTR>(oldStyle & ~WS_VISIBLE));
        TriggerFrameChange(msg->hwnd);
        const LRESULT ret = DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        SetWindowLongPtrW(msg->hwnd, GWL_STYLE, oldStyle);
        TriggerFrameChange(msg->hwnd);
        *result = ret;
        return true;
    }
    case WM_LBUTTONDOWN: {
        const auto xPos = GET_X_LPARAM(msg->lParam);
        const auto yPos = GET_Y_LPARAM(msg->lParam);
        SystemButtonEvent event = {};
        event.window = this;
        event.type = MouseEventType::Down;
        event.mousePos.type = MousePosType::Client;
        event.mousePos.pos = QPointF(qreal(xPos), qreal(yPos));
        const auto hitTestResult = handleSystemButtonEvent(&event);
        if (hitTestResult.has_value()) {
            *result = hitTestResult.value();
            return true;
        }
    } break;
    case WM_LBUTTONUP: {
        const auto xPos = GET_X_LPARAM(msg->lParam);
        const auto yPos = GET_Y_LPARAM(msg->lParam);
        SystemButtonEvent event = {};
        event.window = this;
        event.type = MouseEventType::Up;
        event.mousePos.type = MousePosType::Client;
        event.mousePos.pos = QPointF(qreal(xPos), qreal(yPos));
        const auto hitTestResult = handleSystemButtonEvent(&event);
        if (hitTestResult.has_value()) {
            *result = hitTestResult.value();
            return true;
        }
    } break;
    case WM_NCLBUTTONDOWN: {
        const auto xPos = GET_X_LPARAM(msg->lParam);
        const auto yPos = GET_Y_LPARAM(msg->lParam);
        SystemButtonEvent event = {};
        event.window = this;
        event.type = MouseEventType::Down;
        event.mousePos.type = MousePosType::Screen;
        event.mousePos.pos = QPointF(qreal(xPos), qreal(yPos));
        const auto hitTestResult = handleSystemButtonEvent(&event);
        if (hitTestResult.has_value()) {
            *result = hitTestResult.value();
            return true;
        }
    } break;
    case WM_NCLBUTTONUP: {
        const auto xPos = GET_X_LPARAM(msg->lParam);
        const auto yPos = GET_Y_LPARAM(msg->lParam);
        SystemButtonEvent event = {};
        event.window = this;
        event.type = MouseEventType::Up;
        event.mousePos.type = MousePosType::Screen;
        event.mousePos.pos = QPointF(qreal(xPos), qreal(yPos));
        const auto hitTestResult = handleSystemButtonEvent(&event);
        if (hitTestResult.has_value()) {
            *result = hitTestResult.value();
            return true;
        }
    } break;
    case WM_MOUSEMOVE: {
        const auto xPos = GET_X_LPARAM(msg->lParam);
        const auto yPos = GET_Y_LPARAM(msg->lParam);
        SystemButtonEvent event = {};
        event.window = this;
        event.type = MouseEventType::Hover;
        event.mousePos.type = MousePosType::Client;
        event.mousePos.pos = QPointF(qreal(xPos), qreal(yPos));
        const auto hitTestResult = handleSystemButtonEvent(&event);
        if (hitTestResult.has_value()) {
            *result = hitTestResult.value();
            return true;
        }
    } break;
    case WM_NCMOUSEMOVE: {
        const auto xPos = GET_X_LPARAM(msg->lParam);
        const auto yPos = GET_Y_LPARAM(msg->lParam);
        SystemButtonEvent event = {};
        event.window = this;
        event.type = MouseEventType::Hover;
        event.mousePos.type = MousePosType::Screen;
        event.mousePos.pos = QPointF(qreal(xPos), qreal(yPos));
        const auto hitTestResult = handleSystemButtonEvent(&event);
        if (hitTestResult.has_value()) {
            *result = hitTestResult.value();
            return true;
        }
    } break;
    default:
        break;
    }
#endif
    return false;
}

void FramelessWindow::mouseMoveEvent(QMouseEvent *event)
{
    QQuickWindow::mouseMoveEvent(event);
    if (g_usePureQtImplementation) {
        const Qt::CursorShape cs = calculateCursorShape(this, event->position());
        if (cs == Qt::ArrowCursor) {
            unsetCursor();
        } else {
            setCursor(cs);
        }
    }
}

void FramelessWindow::mousePressEvent(QMouseEvent *event)
{
    QQuickWindow::mousePressEvent(event);
    if (g_usePureQtImplementation) {
        const Qt::Edges edges = calculateWindowEdges(this, event->position());
        if (edges != Qt::Edges{}) {
            if (!startSystemResize2(edges)) {
                qWarning() << "Current platform doesn't support \"QWindow::startSystemResize()\"!";
            }
        }
    }
}

void FramelessWindow::initialize()
{
#ifdef Q_OS_WINDOWS
    if (!g_usePureQtImplementation) {
        const auto hwnd = reinterpret_cast<HWND>(winId());
        Q_ASSERT(hwnd);
        if (hwnd) {
            FixupQtInternals(hwnd);
            UpdateWindowFrameMargins(hwnd);
        }
        // It's necessary to modify the internal frame recorded in QPA, if you
        // don't do this, you'll find the window size is not correct everytime
        // you change the window geometry, because Qt will always take the frame
        // size into account. This issue happens because we remove the window
        // frame by intercepting Windows messages instead of using Qt's own
        // window flags (mainly Qt::FramelessWindowHint), so actually Qt is not
        // aware of the fact that the window doesn't have a window frame now.
        UpdateQtInternalFrame(this);
    }
#endif
    if (g_usePureQtImplementation) {
        setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint
                 | Qt::WindowTitleHint| Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    }
}
