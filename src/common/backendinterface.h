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

#include "../loader/qtmediaplayer_global.h"
#include <QtQuick/qsgrendererinterface.h>

QTMEDIAPLAYER_BEGIN_NAMESPACE

[[maybe_unused]] static const QString kName = QStringLiteral("name");
[[maybe_unused]] static const QString kVersion = QStringLiteral("version");
[[maybe_unused]] static const QString kAuthors = QStringLiteral("authors");
[[maybe_unused]] static const QString kCopyright = QStringLiteral("copyright");
[[maybe_unused]] static const QString kLicenses = QStringLiteral("licenses");
[[maybe_unused]] static const QString kHomepage = QStringLiteral("homepage");
[[maybe_unused]] static const QString kLastModifyTime = QStringLiteral("last-modify-time");
[[maybe_unused]] static const QString kLogo = QStringLiteral("logo");
[[maybe_unused]] static const QString kFFmpegVersion = QStringLiteral("ffmpeg-version");
[[maybe_unused]] static const QString kFFmpegConfiguration = QStringLiteral("ffmpeg-configuration");

class QMPBackend
{
    Q_DISABLE_COPY_MOVE(QMPBackend)

public:
    explicit QMPBackend() = default;
    virtual ~QMPBackend() = default;

    inline void Release()
    {
        delete this;
    }

    [[nodiscard]] virtual QString name() const = 0;
    [[nodiscard]] virtual QString version() const = 0;
    [[nodiscard]] virtual bool available() const = 0;
    [[nodiscard]] virtual bool isRHIBackendSupported(const QSGRendererInterface::GraphicsApi api) const = 0;
    [[nodiscard]] virtual QString filePath() const = 0;
    [[nodiscard]] virtual QString fileName() const = 0;
    [[nodiscard]] virtual bool initialize() const = 0;
};

QTMEDIAPLAYER_END_NAMESPACE
