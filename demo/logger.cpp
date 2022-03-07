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

#include "logger.h"
#include <cstdio>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qmutex.h>
#ifdef Q_OS_WINDOWS
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#    include <QtCore/qoperatingsystemversion.h>
#  else // (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#    include <QtCore/qsysinfo.h>
#  endif // (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
#  include <QtCore/qt_windows.h>
#endif // Q_OS_WINDOWS

enum class ConsoleTextColor : int
{
    Default = 0,
    Black   = 1,
    Red     = 2,
    Green   = 3,
    Yellow  = 4,
    Blue    = 5,
    Magenta = 6,
    Cyan    = 7,
    White   = 8
};

static constexpr const int kVirtualTerminalForegroundColor[] = {0, 30, 31, 32, 33, 34, 35, 36, 37};

#ifdef Q_OS_WINDOWS
static constexpr const WORD kClassicForegroundColor[] =
{
    0, FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_GREEN | FOREGROUND_RED, FOREGROUND_BLUE,
    FOREGROUND_BLUE | FOREGROUND_RED, FOREGROUND_BLUE | FOREGROUND_GREEN,
    FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED
};
#endif

#ifdef Q_OS_WINDOWS
[[nodiscard]] static inline bool isWin10OrGreater()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10);
#else
    static const bool result = (QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10);
#endif
    return result;
}
#endif

[[nodiscard]] static inline bool isVirtualTerminalSequencesSupported()
{
    static const bool result = []() -> bool {
#ifdef Q_OS_WINDOWS
        return isWin10OrGreater();
#else
        return true; // ### TODO
#endif
    }();
    return result;
}

static inline void writeConsole(const QString &text, const ConsoleTextColor color, const bool error)
{
    Q_ASSERT(!text.isEmpty());
    if (text.isEmpty()) {
        return;
    }
    // ### TODO: std::cout/cerr?
    FILE * const channel = (error ? stderr : stdout);
    if (isVirtualTerminalSequencesSupported()) {
        std::fprintf(channel, "\x1b[%dm%s\x1b[0m\n", kVirtualTerminalForegroundColor[static_cast<int>(color)], qPrintable(text));
    } else {
#ifdef Q_OS_WINDOWS
        const HANDLE hCon = GetStdHandle(error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        SecureZeroMemory(&csbi, sizeof(csbi));
        GetConsoleScreenBufferInfo(hCon, &csbi);
        const WORD originalColor = csbi.wAttributes;
        const WORD newColor = ((color == ConsoleTextColor::Default) ? 0 :
                              (kClassicForegroundColor[static_cast<int>(color)] | FOREGROUND_INTENSITY));
        SetConsoleTextAttribute(hCon, newColor | (originalColor & 0xF0));
        std::fprintf(channel, "%s\n", qPrintable(text));
        SetConsoleTextAttribute(hCon, originalColor);
#else
        std::fprintf(channel, "%s\n", qPrintable(text));
#endif
    }
}

class LoggerHelper
{
    Q_DISABLE_COPY_MOVE(LoggerHelper)

public:
    explicit LoggerHelper() {
        initialize();
    }
    ~LoggerHelper() = default;

    inline void append(const QString &text) {
        if (m_available && !text.isEmpty()) {
            m_stream << text << Qt::endl;
        }
    }

private:
    inline void initialize() {
        const QString logDirPath = QCoreApplication::applicationDirPath() + QStringLiteral("/logs");
        const QDir logDir(logDirPath);
        if (!logDir.exists()) {
            writeConsole(QStringLiteral("Log directory doesn't exist. Creating a new one ..."), ConsoleTextColor::Default, false);
            if (!logDir.mkpath(QChar(u'.'))) {
                writeConsole(QStringLiteral("Failed to create the log directory."), ConsoleTextColor::Yellow, true);
                return;
            }
        }
        const QString logFileName = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMddhhmmsszzz")) + QStringLiteral(".log");
        const QString logFilePath = logDirPath + QDir::separator() + logFileName;
        m_file.setFileName(logFilePath);
        if (!m_file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
            writeConsole(QStringLiteral("Failed to open file to write: %1").arg(m_file.errorString()), ConsoleTextColor::Yellow, true);
            return;
        }
        m_stream.setDevice(&m_file);
        m_available = true;
    }

public:
    QMutex m_mutex = {};
    QtMessageHandler m_originalMessageHandler = nullptr;
    bool m_alreadySetup = false;

private:
    QFile m_file;
    QTextStream m_stream = {};
    bool m_available = false;
};

Q_GLOBAL_STATIC(LoggerHelper, g_loggerHelper)

static inline void customMessageHandler(const QtMsgType type, const QMessageLogContext &context, const QString &buf)
{
    if (g_loggerHelper.isDestroyed()) {
        return;
    }
    QMutexLocker locker(&g_loggerHelper()->m_mutex);
    if (buf.isEmpty()) {
        return;
    }
    const QString originalLogMessage = qFormatLogMessage(type, context, buf).trimmed();
    if (originalLogMessage.isEmpty()) {
        return;
    }
    ConsoleTextColor textColor = ConsoleTextColor::Default;
    bool isError = true;
    switch (type) {
    case QtInfoMsg:
        textColor = ConsoleTextColor::Green;
        isError = false;
        break;
    case QtDebugMsg:
        isError = false;
        break;
    case QtWarningMsg:
        textColor = ConsoleTextColor::Yellow;
        break;
    case QtCriticalMsg:
        textColor = ConsoleTextColor::Red;
        break;
    case QtFatalMsg:
        textColor = ConsoleTextColor::Magenta;
        break;
    }
    writeConsole(originalLogMessage, textColor, isError);
    g_loggerHelper()->append(originalLogMessage);
}

void Logger::setup()
{
    QMutexLocker locker(&g_loggerHelper()->m_mutex);
    if (g_loggerHelper()->m_alreadySetup) {
        return;
    }
    g_loggerHelper()->m_alreadySetup = true;
    g_loggerHelper()->m_originalMessageHandler = qInstallMessageHandler(customMessageHandler);
    qSetMessagePattern(QStringLiteral(
        "[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{if-info}INFO%{endif}%{if-debug}DEBUG%{endif}"
        "%{if-warning}WARNING%{endif}%{if-critical}CRITICAL%{endif}%{if-fatal}FATAL%{endif}] "
        "%{if-category}%{category}: %{endif}%{message}"));
}
