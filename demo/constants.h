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

class Constants : public QObject
{
    Q_OBJECT
#ifdef QML_ELEMENT
    QML_ELEMENT
#endif
#ifdef QML_SINGLETON
    QML_SINGLETON
#endif
    Q_DISABLE_COPY_MOVE(Constants)
    Q_PROPERTY(qreal defaultWindowWidth READ defaultWindowWidth CONSTANT FINAL)
    Q_PROPERTY(qreal defaultWindowHeight READ defaultWindowHeight CONSTANT FINAL)
    Q_PROPERTY(qreal resizeBorderThickness READ resizeBorderThickness CONSTANT FINAL)
    Q_PROPERTY(qreal titleBarHeight READ titleBarHeight CONSTANT FINAL)
    Q_PROPERTY(qreal thinTitleBarHeight READ thinTitleBarHeight CONSTANT FINAL)
    Q_PROPERTY(qreal titleBarHideDuration READ titleBarHideDuration CONSTANT FINAL)
    Q_PROPERTY(qreal controlPanelHideDuration READ controlPanelHideDuration CONSTANT FINAL)
    Q_PROPERTY(qreal messageAutoHideDuration READ messageAutoHideDuration CONSTANT FINAL)
    Q_PROPERTY(qreal cursorAutoHideDuration READ cursorAutoHideDuration CONSTANT FINAL)
    Q_PROPERTY(qreal sliderHandleDiameter READ sliderHandleDiameter CONSTANT FINAL)
    Q_PROPERTY(qreal sliderHandleBorderThickness READ sliderHandleBorderThickness CONSTANT FINAL)
    Q_PROPERTY(qreal sliderNormalHeight READ sliderNormalHeight CONSTANT FINAL)
    Q_PROPERTY(qreal sliderHoverHeight READ sliderHoverHeight CONSTANT FINAL)
    Q_PROPERTY(qreal sliderHoverDuration READ sliderHoverDuration CONSTANT FINAL)
    Q_PROPERTY(qreal sliderHandleHoverDuration READ sliderHandleHoverDuration CONSTANT FINAL)
    Q_PROPERTY(qreal progressIndicatorHeight READ progressIndicatorHeight CONSTANT FINAL)
    Q_PROPERTY(qreal titleBarBackgroundHideDuration READ titleBarBackgroundHideDuration CONSTANT FINAL)
    Q_PROPERTY(qreal windowFrameBorderThickness READ windowFrameBorderThickness CONSTANT FINAL)

public:
    explicit Constants(QObject *parent = nullptr);
    ~Constants() override;

    [[nodiscard]] qreal defaultWindowWidth() const;
    [[nodiscard]] qreal defaultWindowHeight() const;
    [[nodiscard]] qreal resizeBorderThickness() const;
    [[nodiscard]] qreal titleBarHeight() const;
    [[nodiscard]] qreal thinTitleBarHeight() const;
    [[nodiscard]] qreal titleBarHideDuration() const;
    [[nodiscard]] qreal controlPanelHideDuration() const;
    [[nodiscard]] qreal messageAutoHideDuration() const;
    [[nodiscard]] qreal cursorAutoHideDuration() const;
    [[nodiscard]] qreal sliderHandleDiameter() const;
    [[nodiscard]] qreal sliderHandleBorderThickness() const;
    [[nodiscard]] qreal sliderNormalHeight() const;
    [[nodiscard]] qreal sliderHoverHeight() const;
    [[nodiscard]] qreal sliderHoverDuration() const;
    [[nodiscard]] qreal sliderHandleHoverDuration() const;
    [[nodiscard]] qreal progressIndicatorHeight() const;
    [[nodiscard]] qreal titleBarBackgroundHideDuration() const;
    [[nodiscard]] qreal windowFrameBorderThickness() const;
};
