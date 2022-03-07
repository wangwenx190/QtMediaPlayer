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
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#    include <QtCore/qoperatingsystemversion.h>
#  else
#    include <QtCore/qsysinfo.h>
#  endif
#  include <QtCore/qmutex.h>
#  include <QtCore/qt_windows.h>
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

#ifdef Q_OS_WINDOWS
static const QString kThemeChangeEventName = QStringLiteral("ImmersiveColorSet");

[[nodiscard]] static inline bool IsWin101809OrGreater()
{
    // Windows 10 Version 1809 (October 2018 Update)
    // Codename RedStone5, version number 10.0.17763
    static const bool result = []() -> bool {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
        return (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10_1809);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
        return (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 17763));
#else
        return false;
#endif
    }();
    return result;
}

[[nodiscard]] static inline LRESULT CALLBACK MsgWndProc
    (const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
    if (message == WM_NCCREATE) {
        const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        const auto theme = static_cast<Theme *>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(theme));
    } else if (message == WM_NCDESTROY) {
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
    }
    if (IsWin101809OrGreater()) {
        if (message == WM_SETTINGCHANGE) {
            if ((wParam == 0) && (QString::compare(QString::fromWCharArray(reinterpret_cast<LPCWSTR>(lParam)), kThemeChangeEventName, Qt::CaseInsensitive) == 0)) {
                if (const auto theme = reinterpret_cast<Theme *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
                    theme->refresh();
                }
            }
        }
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
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
    Q_EMIT darkModeEnabledChanged();
    Q_EMIT titleBarBackgroundColorChanged();
    Q_EMIT windowBackgroundColorChanged();
    Q_EMIT themeColorChanged();
    Q_EMIT systemColorChanged();
    Q_EMIT sliderBackgroundColorChanged();
    Q_EMIT sliderHandleBorderColorChanged();
}
