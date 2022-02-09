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

import QtQuick
import QtQuick.Controls
import wangwenx190.QtMediaPlayer.Demo

Slider {
    signal previewPositionChanged(mouseX: real)
    signal shouldHidePreviewWindow()

    id: slider

    states: [
        State {
            name: "hovered"

            PropertyChanges {
                target: backgroundRect
                height: Constants.sliderHoverHeight
            }

            PropertyChanges {
                target: handleRect
                implicitWidth: Constants.sliderHandleDiameter
            }
        },
        State {
            name: ""

            PropertyChanges {
                target: backgroundRect
                height: Constants.sliderNormalHeight
            }

            PropertyChanges {
                target: handleRect
                implicitWidth: 0
            }
        }
    ]
    transitions: [
        Transition {
            from: ""
            to: "hovered"

            ParallelAnimation {
                PropertyAnimation {
                    target: backgroundRect
                    property: "height"
                    duration: Constants.sliderHoverDuration
                }

                PropertyAnimation {
                    target: handleRect
                    property: "implicitWidth"
                    duration: Constants.sliderHoverDuration
                }
            }
        },
        Transition {
            from: "hovered"
            to: ""

            ParallelAnimation {
                PropertyAnimation {
                    target: backgroundRect
                    property: "height"
                    duration: Constants.sliderHoverDuration
                }

                PropertyAnimation {
                    target: handleRect
                    property: "implicitWidth"
                    duration: Constants.sliderHoverDuration
                }
            }
        }
    ]

    background: Rectangle {
        id: backgroundRect
        x: slider.leftPadding
        y: slider.topPadding + slider.availableHeight / 2 - height / 2
        implicitWidth: 200
        implicitHeight: Constants.sliderNormalHeight
        width: slider.availableWidth
        height: implicitHeight
        radius: 2
        color: Theme.sliderBackgroundColor

        Rectangle {
            width: slider.visualPosition * parent.width
            height: parent.height
            color: Theme.themeColor
            radius: parent.radius
        }
    }

    handle: Rectangle {
        id: handleRect
        x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
        y: slider.topPadding + slider.availableHeight / 2 - height / 2
        implicitWidth: 0
        implicitHeight: implicitWidth
        radius: implicitHeight / 2
        color: Theme.themeColor
        border {
            id: handleBorderRect
            width: Constants.sliderHandleBorderThickness
            color: Theme.sliderHandleBorderColor
        }
        states: [
            State {
                name: "hovered"

                PropertyChanges {
                    target: handleBorderRect
                    width: 2
                }
            },
            State {
                name: ""

                PropertyChanges {
                    target: handleBorderRect
                    width: Constants.sliderHandleBorderThickness
                }
            }
        ]
        transitions: [
            Transition {
                from: ""
                to: "hovered"

                PropertyAnimation {
                    target: handleBorderRect
                    property: "width"
                    duration: Constants.sliderHandleHoverDuration
                }
            },
            Transition {
                from: "hovered"
                to: ""

                PropertyAnimation {
                    target: handleBorderRect
                    property: "width"
                    duration: Constants.sliderHandleHoverDuration
                }
            }
        ]
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton // Don't handle actual events.
        hoverEnabled: true
        onEntered: slider.state = "hovered"
        onExited: {
            slider.state = "";
            if (Settings.enableTimelinePreview) {
                slider.shouldHidePreviewWindow();
            }
        }
        onMouseXChanged: {
            if (Settings.enableTimelinePreview) {
                slider.previewPositionChanged(mouseX);
            }
            if ((mouseX >= handleRect.x) && (mouseX <= (handleRect.x + handleRect.width))) {
                handleRect.state = "hovered";
            } else {
                handleRect.state = "";
            }
        }
    }
}
