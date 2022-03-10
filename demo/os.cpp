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

#include "os.h"
#include <QtCore/qsysinfo.h>
#include <QtCore/qoperatingsystemversion.h>

[[nodiscard]] static inline OS::Arch stringToArch(const QString &str)
{
    Q_ASSERT(!str.isEmpty());
    if (str.isEmpty()) {
        return OS::Arch::Unknown;
    }
    if (QString::compare(str, QStringLiteral("x86_64"), Qt::CaseInsensitive) == 0) {
        return OS::Arch::X64;
    }
    if (QString::compare(str, QStringLiteral("i386"), Qt::CaseInsensitive) == 0) {
        return OS::Arch::X86;
    }
    if (QString::compare(str, QStringLiteral("arm64"), Qt::CaseInsensitive) == 0) {
        return OS::Arch::ARM64;
    }
    if (QString::compare(str, QStringLiteral("arm"), Qt::CaseInsensitive) == 0) {
        return OS::Arch::ARM;
    }
    return OS::Arch::Unknown;
}

OS::OS(QObject *parent) : QObject(parent)
{
}

OS::~OS() = default;

bool OS::isWindowsHost() const
{
#ifdef Q_OS_WINDOWS
    return true;
#else
    return false;
#endif
}

bool OS::isLinuxHost() const
{
#ifdef Q_OS_LINUX
    return true;
#else
    return false;
#endif
}

bool OS::isMacOSHost() const
{
#ifdef Q_OS_MACOS
    return true;
#else
    return false;
#endif
}

bool OS::isAndroidHost() const
{
#ifdef Q_OS_ANDROID
    return true;
#else
    return false;
#endif
}

bool OS::isIOSHost() const
{
#ifdef Q_OS_IOS
    return true;
#else
    return false;
#endif
}

bool OS::isDesktopHost() const
{
    return (isWindowsHost() || (isLinuxHost() && !isAndroidHost()) || isMacOSHost());
}

bool OS::isMobileHost() const
{
    return (isAndroidHost() || isIOSHost());
}

bool OS::isEmbededHost() const
{
    // ### TODO
    return false;
}

OS::Arch OS::buildCpuArchitecture() const
{
    static const Arch result = stringToArch(QSysInfo::buildCpuArchitecture());
    return result;
}

OS::Arch OS::currentCpuArchitecture() const
{
    static const Arch result = stringToArch(QSysInfo::currentCpuArchitecture());
    return result;
}

QString OS::version() const
{
    static const QString result = QSysInfo::productVersion();
    return result;
}

bool OS::isWindows10OrGreater() const
{
#ifdef Q_OS_WINDOWS
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10);
    return result;
#else
    return false;
#endif
}

bool OS::isWindows11OrGreater() const
{
#ifdef Q_OS_WINDOWS
#if (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows11);
#else
    static const bool result = (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 22000));
#endif
    return result;
#else
    return false;
#endif
}
