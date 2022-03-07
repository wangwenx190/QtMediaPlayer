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

#include <QtQuick/qquickwindow.h>

class FramelessWindow : public QQuickWindow
{
    Q_OBJECT
#ifdef QML_NAMED_ELEMENT
    QML_NAMED_ELEMENT(FramelessWindow)
#endif
    Q_DISABLE_COPY_MOVE(FramelessWindow)

public:
    explicit FramelessWindow(QWindow *parent = nullptr);
    ~FramelessWindow() override;

    [[nodiscard]] Q_INVOKABLE bool isHidden() const;
    [[nodiscard]] Q_INVOKABLE bool isMinimized() const;
    [[nodiscard]] Q_INVOKABLE bool isMaximized() const;
    [[nodiscard]] Q_INVOKABLE bool isFullScreen() const;

public Q_SLOTS:
    void showMinimized2();
    void toggleMaximized();
    void toggleFullScreen();
    void bringToFront();
    void moveToCenter();
    [[nodiscard]] bool startSystemResize2(const Qt::Edges edges);
    [[nodiscard]] bool startSystemMove2();
    void zoomIn(const qreal step);
    void zoomOut(const qreal step);

protected:
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    [[nodiscard]] bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    [[nodiscard]] bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void initialize();

private:
    Visibility m_savedVisibility = Windowed;
};
