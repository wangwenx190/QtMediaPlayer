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
import QtQuick.Layouts

Item {
    property alias playing: playButton.playing
    property alias time: timeLabel.text
    property alias duration: durationLabel.text
    property alias positionSlider: positionSlider
    property alias seekable: positionSlider.enabled
    property alias volume: volumeSlider.value
    property alias mute: volumeButton.mute

    signal playButtonClicked()
    signal stopButtonClicked()
    signal volumeButtonClicked()
    signal shouldUpdateVolume(newVolume: real)

    id: panel
    implicitWidth: 200
    implicitHeight: 30

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 5

        Label {
            id: timeLabel
            font {
                bold: true
                pointSize: 11
            }
            color: Qt.color("white")
        }

        PlayButton {
            id: playButton
            width: 30
            height: width
            onClicked: playButtonClicked()
        }

        StopButton {
            id: stopButton
            width: 30
            height: width
            onClicked: stopButtonClicked()
        }

        VolumeButton {
            id: volumeButton
            width: 30
            height: width
            onClicked: volumeButtonClicked()
        }

        ProgressSlider {
            id: volumeSlider
            implicitWidth: 70
            onMoved: shouldUpdateVolume(value)
        }

        ProgressSlider {
            id: positionSlider
            Layout.fillWidth: true
        }

        Label {
            id: durationLabel
            font {
                bold: true
                pointSize: 11
            }
            color: Qt.color("white")
        }
    }
}
