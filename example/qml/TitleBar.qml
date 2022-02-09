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

Rectangle {
    property alias text: windowTitleLabel.text
    property alias maximized: maximizeButton.maximized

    property bool fullscreened: false

    signal minimizeButtonClicked()
    signal maximizeButtonClicked()
    signal closeButtonClicked()

    id: titleBar
    implicitWidth: 100
    implicitHeight: Constants.titleBarHeight
    color: Theme.titleBarBackgroundColor
    states: [
        State {
            name: "hide_background"

            PropertyChanges {
                target: titleBar
                color: Qt.color("transparent")
            }
        }
    ]
    transitions: [
        Transition {
            from: ""
            to: "hide_background"

            ColorAnimation {
                target: titleBar
                property: "color"
                duration: Constants.titleBarBackgroundHideDuration
            }
        },
        Transition {
            from: "hide_background"
            to: ""

            ColorAnimation {
                target: titleBar
                property: "color"
                duration: Constants.titleBarBackgroundHideDuration
            }
        }
    ]

    Label {
        id: windowTitleLabel
        anchors.centerIn: parent
        color: Theme.systemColor
    }

    Row {
        visible: !titleBar.fullscreened
        anchors {
            top: parent.top
            right: parent.right
        }

        MinimizeButton {
            height: titleBar.height
            width: height * 1.5
            onClicked: minimizeButtonClicked()
        }

        MaximizeButton {
            id: maximizeButton
            height: titleBar.height
            width: height * 1.5
            onClicked: maximizeButtonClicked()
        }

        CloseButton {
            height: titleBar.height
            width: height * 1.5
            onClicked: closeButtonClicked()
        }
    }
}
