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

#include "theme.h"
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 1))
#  include <QtGui/qpa/qplatformtheme.h>
#  include <QtGui/private/qguiapplication_p.h>
#endif
#ifdef Q_OS_WINDOWS
#  include <QtCore/qoperatingsystemversion.h>
#  include <QtCore/qmutex.h>
#  include <QtCore/private/qsystemlibrary_p.h>
#  include <QtCore/private/qwinregistry_p.h>
#  include <QtCore/qt_windows.h>
#  include <dwmapi.h>
#endif

#ifdef Q_OS_WINDOWS
#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#  define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

enum class DwmColorizationArea : int
{
    None = 0,
    StartMenu_TaskBar_ActionCenter = 1,
    TitleBar_WindowBorder = 2,
    All = 3
};

static const QString kThemeSettingChangeEventName = QStringLiteral("ImmersiveColorSet");
static const QString kDwmRegistryKey = QStringLiteral(R"(Software\Microsoft\Windows\DWM)");
static const QString kPersonalizeRegistryKey = QStringLiteral(R"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)");

[[nodiscard]] static inline bool IsWin10OrGreater()
{
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10);
    return result;
}

[[nodiscard]] static inline bool IsWin101809OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10_1809);
#else
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 17763));
#endif
    return result;
}

[[nodiscard]] static inline QColor GetDwmColorizationColor()
{
    static const auto pDwmGetColorizationColor =
        reinterpret_cast<decltype(&DwmGetColorizationColor)>(
            QSystemLibrary::resolve(QStringLiteral("dwmapi"), "DwmGetColorizationColor"));
    if (!pDwmGetColorizationColor) {
        return QColorConstants::DarkGray;
    }
    DWORD color = 0;
    BOOL opaque = FALSE;
    pDwmGetColorizationColor(&color, &opaque);
    return QColor::fromRgba(color);
}

[[nodiscard]] static inline DwmColorizationArea GetDwmColorizationArea()
{
    if (!IsWin10OrGreater()) {
        return DwmColorizationArea::None;
    }
    static const QString keyName = QStringLiteral("ColorPrevalence");
    const QWinRegistryKey themeRegistry(HKEY_CURRENT_USER, kPersonalizeRegistryKey);
    const auto themeValue = themeRegistry.dwordValue(keyName);
    const QWinRegistryKey dwmRegistry(HKEY_CURRENT_USER, kDwmRegistryKey);
    const auto dwmValue = dwmRegistry.dwordValue(keyName);
    const bool theme = themeValue.second && (themeValue.first != 0);
    const bool dwm = dwmValue.second && (dwmValue.first != 0);
    if (theme && dwm) {
        return DwmColorizationArea::All;
    } else if (theme) {
        return DwmColorizationArea::StartMenu_TaskBar_ActionCenter;
    } else if (dwm) {
        return DwmColorizationArea::TitleBar_WindowBorder;
    }
    return DwmColorizationArea::None;
}

[[nodiscard]] static inline LRESULT CALLBACK MsgWndProc
    (const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    if (uMsg == WM_NCCREATE) {
        const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        const auto theme = static_cast<Theme *>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(theme));
    } else if (uMsg == WM_NCDESTROY) {
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
    }
    if (IsWin10OrGreater()) {
        bool shouldRefresh = false;
        if ((uMsg == WM_THEMECHANGED) || (uMsg == WM_DWMCOLORIZATIONCOLORCHANGED)) {
            shouldRefresh = true;
        }
        if (IsWin101809OrGreater()) {
            if (uMsg == WM_SETTINGCHANGE) {
                if ((wParam == 0) && (QString::fromWCharArray(reinterpret_cast<LPCWSTR>(lParam)).compare(kThemeSettingChangeEventName, Qt::CaseInsensitive) == 0)) {
                    shouldRefresh = true;
                }
            }
        }
        if (shouldRefresh) {
            if (const auto theme = reinterpret_cast<Theme *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
                theme->refresh();
            }
        }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

struct Win32SystemThemeWatcherHelper
{
    QMutex mutex = {};

    HINSTANCE instance = nullptr;
    HWND windowHandle = nullptr;
    static constexpr const wchar_t className[] = L"QtMediaPlayerDemoAppMsgWndCls";

    explicit Win32SystemThemeWatcherHelper() = default;
    ~Win32SystemThemeWatcherHelper() = default;

private:
    Q_DISABLE_COPY_MOVE(Win32SystemThemeWatcherHelper)
};

Q_GLOBAL_STATIC(Win32SystemThemeWatcherHelper, g_globalWatcher)

static inline void setupSystemThemeWatcher(Theme *theme)
{
    Q_ASSERT(theme);
    if (!theme) {
        return;
    }
    QMutexLocker locker(&g_globalWatcher()->mutex);
    if (g_globalWatcher()->windowHandle) {
        return;
    }
    g_globalWatcher()->instance = static_cast<HINSTANCE>(GetModuleHandleW(nullptr));
    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.lpfnWndProc = MsgWndProc;
    wcex.hInstance = g_globalWatcher()->instance;
    wcex.lpszClassName = g_globalWatcher()->className;
    RegisterClassExW(&wcex);
    g_globalWatcher()->windowHandle = CreateWindowExW(
        0, g_globalWatcher()->className, nullptr, 0, 0, 0, 0, 0,
        nullptr, nullptr, g_globalWatcher()->instance, theme);
    Q_ASSERT(g_globalWatcher()->windowHandle);
}

static inline void removeSystemThemeWatcher(Theme *theme)
{
    Q_UNUSED(theme);
    QMutexLocker locker(&g_globalWatcher()->mutex);
    if (!g_globalWatcher()->windowHandle) {
        return;
    }
    DestroyWindow(g_globalWatcher()->windowHandle);
    g_globalWatcher()->windowHandle = nullptr;
    UnregisterClassW(g_globalWatcher()->className, g_globalWatcher()->instance);
    g_globalWatcher()->instance = nullptr;
}
#else
static inline void setupSystemThemeWatcher(Theme *theme)
{
    Q_UNUSED(theme);
}

static inline void removeSystemThemeWatcher(Theme *theme)
{
    Q_UNUSED(theme);
}
#endif

[[nodiscard]] static inline bool _qt_is_darkmode_enabled()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 1))
    if (const QPlatformTheme * const theme = QGuiApplicationPrivate::platformTheme()) {
        return (theme->appearance() == QPlatformTheme::Appearance::Dark);
    }
    return false;
#else
    return false;
#endif
}

Theme::Theme(QObject *parent) : QObject(parent)
{
    setupSystemThemeWatcher(this);
    refresh();
}

Theme::~Theme()
{
    removeSystemThemeWatcher(this);
}

bool Theme::darkModeEnabled() const
{
    return m_darkModeEnabled;
}

QColor Theme::titleBarBackgroundColor() const
{
    return m_titleBarBackgroundColor;
}

QColor Theme::windowBackgroundColor() const
{
    return m_windowBackgroundColor;
}

QColor Theme::themeColor() const
{
    return m_themeColor;
}

QColor Theme::systemColor() const
{
    return m_systemColor;
}

QColor Theme::sliderBackgroundColor() const
{
    return m_sliderBackgroundColor;
}

QColor Theme::sliderHandleBorderColor() const
{
    return m_sliderHandleBorderColor;
}

QColor Theme::systemAccentColor() const
{
    return m_systemAccentColor;
}

QColor Theme::windowFrameBorderColor() const
{
    return m_windowFrameBorderColor;
}

void Theme::refresh()
{
    m_darkModeEnabled = _qt_is_darkmode_enabled();
    if (m_darkModeEnabled) {
        m_titleBarBackgroundColor = QColor(QStringLiteral("#3c3c3c"));
        m_windowBackgroundColor = QColor(QStringLiteral("#1e1e1e"));
        m_systemColor = QColor(QStringLiteral("#c5c5c5"));
    } else {
        m_titleBarBackgroundColor = QColor(QStringLiteral("#dddddd"));
        m_windowBackgroundColor = QColor(QStringLiteral("#ffffff"));
        m_systemColor = QColor(QStringLiteral("#424242"));
    }
    m_themeColor = QColor(QStringLiteral("#1296db"));
    m_sliderBackgroundColor = QColor(QStringLiteral("#bdbebf"));
    m_sliderHandleBorderColor = QColorConstants::Svg::darkslateblue;
    m_systemAccentColor = GetDwmColorizationColor();
    m_windowFrameBorderColor = [this]() -> QColor {
        const DwmColorizationArea area = GetDwmColorizationArea();
        if ((area == DwmColorizationArea::TitleBar_WindowBorder) || (area == DwmColorizationArea::All)) {
            return m_systemAccentColor;
        }
        if (m_darkModeEnabled) {
            return QColor(QStringLiteral("#4d4d4d"));
        }
        return QColorConstants::Black;
    }();
    Q_EMIT darkModeEnabledChanged();
    Q_EMIT titleBarBackgroundColorChanged();
    Q_EMIT windowBackgroundColorChanged();
    Q_EMIT themeColorChanged();
    Q_EMIT systemColorChanged();
    Q_EMIT sliderBackgroundColorChanged();
    Q_EMIT sliderHandleBorderColorChanged();
    Q_EMIT systemAccentColorChanged();
    Q_EMIT windowFrameBorderColorChanged();
}
