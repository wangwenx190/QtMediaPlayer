/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import Qt.labs.platform 1.1
import org.wangwenx190.QtMediaPlayer 1.0

Window {
    id: window
    visible: true
    width: 800
    height: 600
    title: (mediaPlayer.fileName.length > 0) ? (qsTr("Current playing: ") + mediaPlayer.fileName) : qsTr("QtMediaPlayer demo application")

    Shortcut {
        sequence: StandardKey.Open
        onActivated: fileDialog.open()
    }

    QtMediaPlayer {
        id: mediaPlayer
        anchors.fill: parent
        source: fileDialog.file
        logLevel: QtMediaPlayer.LogLevel.Info
        hardwareDecoding: true
        onVideoSizeChanged: {
            if (window.visibility === Window.Windowed) {
                var vs = mediaPlayer.videoSize;
                if (vs.width > 99) {
                    window.width = vs.width;
                }
                if (vs.height > 99) {
                    window.height = vs.height;
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        currentFile: mediaPlayer.source
        folder: StandardPaths.writableLocation(StandardPaths.MoviesLocation)
        options: FileDialog.ReadOnly
        nameFilters: [
            qsTr("Video files (%1)").arg(mediaPlayer.videoSuffixes.join(' ')),
            qsTr("Audio files (%1)").arg(mediaPlayer.audioSuffixes.join(' ')),
            qsTr("All files (*)")
        ]
    }

    CheckBox {
        id: checkbox
        anchors {
            top: parent.top
            right: parent.right
        }
        text: qsTr("Hardware decoding")
        checked: mediaPlayer.hardwareDecoding
        font {
            bold: true
            pointSize: 15
        }
        onCheckedChanged: mediaPlayer.hardwareDecoding = checkbox.checked

        contentItem: Text {
            text: checkbox.text
            font: checkbox.font
            opacity: checkbox.enabled ? 1.0 : 0.3
            color: checkbox.down ? "#17a81a" : "#21be2b"
            verticalAlignment: Text.AlignVCenter
            leftPadding: checkbox.indicator.width + checkbox.spacing
        }
    }

    Button {
        visible: mediaPlayer.playbackState === QtMediaPlayer.PlaybackState.Stopped
        anchors.centerIn: parent
        width: 180
        height: 80
        text: qsTr("Open file")
        font.pointSize: 14
        onClicked: fileDialog.open()
    }

    Slider {
        id: slider
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            bottomMargin: 20
        }
        visible: mediaPlayer.playbackState !== QtMediaPlayer.PlaybackState.Stopped
        to: mediaPlayer.duration
        value: mediaPlayer.position
        onMoved: mediaPlayer.seek(slider.value)

        background: Rectangle {
            id: slider_background
            x: slider.leftPadding
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            implicitWidth: 200
            implicitHeight: 7
            width: slider.availableWidth
            height: implicitHeight
            radius: 2
            color: "#bdbebf"

            Rectangle {
                width: slider.visualPosition * parent.width
                height: parent.height
                color: "#21be2b"
                radius: 2
            }

            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                onEntered: preview.visible = true
                onExited: preview.visible = false
                onPositionChanged: {
                    var minX = 0;
                    var maxX = window.width - preview.width;
                    var curX = mouseX - (preview.width / 2);
                    var newX = (curX < minX) ? minX : ((curX > maxX) ? maxX : curX);
                    preview.x = newX;
                    var newPos = (mouseX - slider_background.x) / slider_background.width;
                    preview.seek(newPos * mediaPlayer.duration);
                }
            }
        }

        handle: Rectangle {
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            implicitWidth: 20
            implicitHeight: 20
            radius: 10
            color: slider.pressed ? "#f0f0f0" : "#f6f6f6"
            border.color: "#bdbebf"
        }
    }

    QtMediaPlayer {
        id: preview
        visible: false
        source: mediaPlayer.source
        livePreview: true
        hardwareDecoding: true
        logLevel: QtMediaPlayer.LogLevel.Warning
        width: 300
        height: 168.75
        y: window.height - preview.height - 50;

        Label {
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: 5
            }
            color: "white"
            font {
                bold: true
                pointSize: 15
            }
            text: qsTr("00:00:00")
        }
    }
}
