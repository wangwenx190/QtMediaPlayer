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

#include "console.h"
#include <QtCore/qmutex.h>
#ifdef Q_OS_WINDOWS
#  include <cstdio>
#  include <iostream>
#  include <QtCore/qt_windows.h>
#endif

#ifdef Q_OS_WINDOWS
#  ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#    define ENABLE_VIRTUAL_TERMINAL_PROCESSING (0x0004)
#  endif
#endif

struct ConsoleData
{
    QMutex m_mutex = {};
    bool m_consoleCreated = false;

    explicit ConsoleData() = default;
    ~ConsoleData() = default;

private:
    Q_DISABLE_COPY_MOVE(ConsoleData)
};

Q_GLOBAL_STATIC(ConsoleData, g_consoleData)

bool Console::isPresent()
{
    QMutexLocker locker(&g_consoleData()->m_mutex);
    return g_consoleData()->m_consoleCreated;
}

#ifdef Q_OS_WINDOWS
void Console::create()
{
    if (isPresent()) {
        return;
    }
    if (!AllocConsole()) {
        return;
    }
    SetConsoleTitleW(L"QtMediaPlayer Demo debug console");
    // std::cout, std::clog, std::cerr, std::cin
    std::freopen("CONOUT$", "w", stdout);
    std::freopen("CONOUT$", "w", stderr);
    std::freopen("CONIN$", "r", stdin);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();
    // std::wcout, std::wclog, std::wcerr, std::wcin
    const HANDLE hConOut = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    const HANDLE hConIn = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
    SetStdHandle(STD_ERROR_HANDLE, hConOut);
    SetStdHandle(STD_INPUT_HANDLE, hConIn);
    std::wcout.clear();
    std::wclog.clear();
    std::wcerr.clear();
    std::wcin.clear();
    const auto enableVTP = [](const HANDLE handle){
        Q_ASSERT(handle);
        if (!handle) {
            return;
        }
        if (handle == INVALID_HANDLE_VALUE) {
            return;
        }
        DWORD dwMode = 0;
        if (!GetConsoleMode(handle, &dwMode)) {
            return;
        }
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(handle, dwMode);
    };
    enableVTP(hConOut);
    QMutexLocker locker(&g_consoleData()->m_mutex);
    g_consoleData()->m_consoleCreated = true;
}

void Console::close()
{
    if (!isPresent()) {
        return;
    }
    FreeConsole();
}
#else
void Console::create() {}
void Console::close() {}
#endif
