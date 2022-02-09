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

#include "constants.h"

Constants::Constants(QObject *parent) : QObject(parent)
{
}

Constants::~Constants() = default;

qreal Constants::defaultWindowWidth() const
{
    return 640.0;
}

qreal Constants::defaultWindowHeight() const
{
    return 480.0;
}

qreal Constants::resizeBorderThickness() const
{
    return 8.0;
}

qreal Constants::titleBarHeight() const
{
    return 31.0;
}

qreal Constants::thinTitleBarHeight() const
{
    return 23.0;
}

qreal Constants::titleBarHideDuration() const
{
    return 200.0;
}

qreal Constants::controlPanelHideDuration() const
{
    return 200.0;
}

qreal Constants::messageAutoHideDuration() const
{
    return 3000.0;
}

qreal Constants::cursorAutoHideDuration() const
{
    return 3000.0;
}

qreal Constants::sliderHandleDiameter() const
{
    return 18.0;
}

qreal Constants::sliderHandleBorderThickness() const
{
    return 5.0;
}

qreal Constants::sliderNormalHeight() const
{
    return 4.0;
}

qreal Constants::sliderHoverHeight() const
{
    return 9.0;
}

qreal Constants::sliderHoverDuration() const
{
    return 100.0;
}

qreal Constants::sliderHandleHoverDuration() const
{
    return 100.0;
}

qreal Constants::progressIndicatorHeight() const
{
    return 4.0;
}

qreal Constants::titleBarBackgroundHideDuration() const
{
    return 300.0;
}
