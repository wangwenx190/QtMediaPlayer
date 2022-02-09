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

#pragma once

#include <QtCore/qobject.h>
#include <QtQml/qqmlregistration.h>

class OS : public QObject
{
    Q_OBJECT
#ifdef QML_ELEMENT
    QML_ELEMENT
#endif
#ifdef QML_SINGLETON
    QML_SINGLETON
#endif
    Q_DISABLE_COPY_MOVE(OS)
    Q_PROPERTY(bool isWindowsHost READ isWindowsHost CONSTANT FINAL)
    Q_PROPERTY(bool isLinuxHost READ isLinuxHost CONSTANT FINAL)
    Q_PROPERTY(bool isMacOSHost READ isMacOSHost CONSTANT FINAL)
    Q_PROPERTY(bool isAndroidHost READ isAndroidHost CONSTANT FINAL)
    Q_PROPERTY(bool isIOSHost READ isIOSHost CONSTANT FINAL)
    Q_PROPERTY(bool isDesktopHost READ isDesktopHost CONSTANT FINAL)
    Q_PROPERTY(bool isMobileHost READ isMobileHost CONSTANT FINAL)
    Q_PROPERTY(bool isEmbededHost READ isEmbededHost CONSTANT FINAL)
    Q_PROPERTY(Arch buildCpuArchitecture READ buildCpuArchitecture CONSTANT FINAL)
    Q_PROPERTY(Arch currentCpuArchitecture READ currentCpuArchitecture CONSTANT FINAL)
    Q_PROPERTY(QString version READ version CONSTANT FINAL)

public:
    enum class Arch : int
    {
        Unknown = 0,
        X86 = 1,
        X64 = 2,
        ARM = 3,
        ARM64 = 4
    };
    Q_ENUM(Arch)

    explicit OS(QObject *parent = nullptr);
    ~OS() override;

    [[nodiscard]] bool isWindowsHost() const;
    [[nodiscard]] bool isLinuxHost() const;
    [[nodiscard]] bool isMacOSHost() const;
    [[nodiscard]] bool isAndroidHost() const;
    [[nodiscard]] bool isIOSHost() const;
    [[nodiscard]] bool isDesktopHost() const;
    [[nodiscard]] bool isMobileHost() const;
    [[nodiscard]] bool isEmbededHost() const;
    [[nodiscard]] Arch buildCpuArchitecture() const;
    [[nodiscard]] Arch currentCpuArchitecture() const;
    [[nodiscard]] QString version() const;
};
