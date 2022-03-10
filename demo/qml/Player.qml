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
import org.wangwenx190.QtMediaPlayer
import wangwenx190.QtMediaPlayer.Demo

Item {
    property alias fileName: player.fileName
    property alias filePath: player.filePath
    property alias source: player.source
    property alias playbackState: player.playbackState
    property alias videoFileSuffixes: player.videoFileSuffixes
    property alias audioFileSuffixes: player.audioFileSuffixes
    property alias videoFileMimeTypes: player.videoFileMimeTypes
    property alias audioFileMimeTypes: player.audioFileMimeTypes
    property alias recommendedWindowSize: player.recommendedWindowSize
    property alias recommendedWindowPosition: player.recommendedWindowPosition
    property alias volume: player.volume
    property alias mute: player.mute
    property alias position: player.position
    property alias duration: player.duration
    property alias videoSize: player.videoSize
    property alias logLevel: player.logLevel
    property alias hardwareDecoding: player.hardwareDecoding
    property alias snapshotFormat: player.snapshotFormat
    property alias snapshotDirectory: player.snapshotDirectory
    property alias seekable: player.seekable

    property bool windowed: true

    signal loaded()
    signal playing()
    signal paused()
    signal stopped()
    signal requestToggleFullScreen()
    signal requestHideTitleBar()
    signal requestShowTitleBar()
    signal requestMoveWindow()

    function showMessage(message: string) {
        messageLabelTimer.stop();
        messageLabel.text = message;
        messageLabel.visible = true;
        messageLabelTimer.restart();
    }

    function togglePlayPause() {
        if (player.playbackState !== QtMediaPlayer.Stopped) {
            if (player.playbackState === QtMediaPlayer.Playing) {
                player.pause();
            } else {
                player.play();
            }
        }
    }

    function seekBackward() {
        if (player.playbackState !== QtMediaPlayer.Stopped) {
            showMessage(qsTr("Backward 5s"));
            player.position -= 5000;
        }
    }

    function seekForward() {
        if (player.playbackState !== QtMediaPlayer.Stopped) {
            showMessage(qsTr("Forward 5s"));
            player.position += 5000;
        }
    }

    function volumeUp() {
        player.volume += 0.1;
    }

    function volumeDown() {
        player.volume -= 0.1;
    }

    function hideControlPanel() {
        mouseArea.cursorShape = Qt.BlankCursor;
        root.state = "hide_control_panel";
        root.requestHideTitleBar();
    }

    function showControlPanel() {
        mouseArea.cursorShape = Qt.ArrowCursor;
        root.state = "";
        root.requestShowTitleBar();
    }

    function isPlayingVideo() {
        return player.isPlayingVideo();
    }

    function isPlayingAudio() {
        return player.isPlayingAudio();
    }

    function play() {
        player.play();
    }

    function pause() {
        player.pause();
    }

    function stop() {
        player.stop();
    }

    id: root
    states: [
        State {
            name: "hide_control_panel"

            PropertyChanges {
                target: controlPanel
                y: root.height
            }

            PropertyChanges {
                target: progressIndicator
                y: root.height - progressIndicator.height
            }
        }
    ]
    transitions: [
        Transition {
            from: ""
            to: "hide_control_panel"

            ParallelAnimation {
                PropertyAnimation {
                    target: controlPanel
                    property: "y"
                    duration: Constants.controlPanelHideDuration
                }

                PropertyAnimation {
                    target: progressIndicator
                    property: "y"
                    duration: Constants.controlPanelHideDuration
                }
            }
        },
        Transition {
            from: "hide_control_panel"
            to: ""

            ParallelAnimation {
                PropertyAnimation {
                    target: controlPanel
                    property: "y"
                    duration: Constants.controlPanelHideDuration
                }

                PropertyAnimation {
                    target: progressIndicator
                    property: "y"
                    duration: Constants.controlPanelHideDuration
                }
            }
        }
    ]

    Timer {
        id: messageLabelTimer
        interval: Constants.messageAutoHideDuration
        onTriggered: {
            messageLabel.visible = false;
            messageLabel.text = "";
        }
    }

    Timer {
        id: currentLocalTimeLabelTimer
        interval: 500
        running: Settings.osdShowLocalTime
        repeat: true
        onTriggered: currentLocalTimeLabel.text = Qt.formatTime(new Date(), "hh:mm:ss")
    }

    Timer {
        id: autoHideTimer
        interval: Constants.cursorAutoHideDuration
        running: player.playbackState !== QtMediaPlayer.Stopped
        onTriggered: hideControlPanel()
    }

    MediaPlayer {
        id: player
        anchors.fill: parent
        onLoaded: {
            root.loaded();
            if (Settings.startFromLastPosition) {
                position = History.getLastPosition(source);
            }
        }
        onPlaying: {
            showMessage(qsTr("Playing"));
            root.playing();
        }
        onPaused: {
            showMessage(qsTr("Paused"));
            root.paused();
        }
        onStopped: {
            showMessage(qsTr("Stopped"));
            root.stopped();
        }
        onStoppedWithPosition: {
            if (Settings.saveHistory) {
                History.insertNewRecord(url, pos);
            }
        }
        onVolumeChanged: showMessage(qsTr("Volume: %1%").arg(Math.round(volume * 100.0)))
        onMuteChanged: showMessage(mute ? qsTr("Mute") : qsTr("Unmute"))
        onHardwareDecodingChanged: showMessage(qsTr("Hardware decoding: %1").arg(hardwareDecoding ? qsTr("ON") : qsTr("OFF")))
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        anchors.bottomMargin: (root.windowed && !OS.isWindows10OrGreater) ? Constants.resizeBorderThickness : 0
        anchors.leftMargin: (root.windowed && !OS.isWindows10OrGreater) ? Constants.resizeBorderThickness : 0
        anchors.rightMargin: (root.windowed && !OS.isWindows10OrGreater) ? Constants.resizeBorderThickness : 0
        acceptedButtons: Qt.LeftButton
        enabled: player.playbackState !== QtMediaPlayer.Stopped
        hoverEnabled: true
        onPositionChanged: {
            autoHideTimer.stop();
            showControlPanel();
            autoHideTimer.restart();
            if (containsPress && root.windowed) {
                root.requestMoveWindow();
            }
        }
        onClicked: togglePlayPause()
        onDoubleClicked: root.requestToggleFullScreen()
    }

    ControlPanel {
        id: controlPanel
        anchors {
            left: parent.left
            right: parent.right
        }
        y: root.height - controlPanel.height
        playing: player.playbackState === QtMediaPlayer.Playing
        time: player.formatTime(player.position, "hh:mm:ss")
        duration: player.formatTime(player.duration, "hh:mm:ss")
        seekable: player.seekable
        volume: player.volume
        mute: player.mute
        onPlayButtonClicked: togglePlayPause()
        onStopButtonClicked: player.stop()
        onVolumeButtonClicked: player.mute = !player.mute
        onShouldUpdateVolume: player.volume = newVolume
        positionSlider {
            id: positionSlider
            to: player.duration
            value: player.position
            onMoved: {
                showMessage(qsTr("Seek to: %1%").arg(Math.round(positionSlider.value / player.duration * 100.0)));
                player.seek(positionSlider.value);
            }
            onPreviewPositionChanged: {
                if (Settings.enableTimelinePreview) {
                    const percent = (mouseX - positionSlider.leftPadding) / positionSlider.availableWidth;
                    timelinePreviewPlayer.seek(timelinePreviewPlayer.duration * percent);
                    let newX = mouseX + positionSlider.x - (timelinePreviewPlayer.width / 2.0);
                    if (newX < 0) {
                        newX = 0;
                    }
                    const maxX = parent.width - timelinePreviewPlayer.width;
                    if (newX > maxX) {
                        newX = maxX;
                    }
                    timelinePreviewPlayer.x = newX;
                    timelinePreviewPlayer.visible = true;
                }
            }
            onShouldHidePreviewWindow: timelinePreviewPlayer.visible = false
        }
    }

    Label {
        id: messageLabel
        anchors {
            top: parent.top
            topMargin: 10
            left: parent.left
            leftMargin: 10
        }
        font {
            bold: true
            pointSize: 15
        }
        color: Qt.color("white")
    }

    Label {
        id: currentLocalTimeLabel
        visible: Settings.osdShowLocalTime
        anchors {
            top: parent.top
            topMargin: 40
            right: parent.right
            rightMargin: 20
        }
        font {
            bold: true
            pointSize: 15
        }
        color: Qt.color("yellow")
    }

    MediaPlayer {
        id: timelinePreviewPlayer
        enabled: Settings.enableTimelinePreview && (player.playbackState !== QtMediaPlayer.Stopped)
        visible: false
        hardwareDecoding: false
        livePreview: true
        source: player.source
        x: 0
        y: parent.height - controlPanel.height - height
        width: parent.width * Settings.timelinePreviewZoomFactor
        height: parent.height * Settings.timelinePreviewZoomFactor

        Label {
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: 5
            }
            color: Qt.color("white")
            font {
                bold: true
                pointSize: 15
            }
            text: timelinePreviewPlayer.formatTime(timelinePreviewPlayer.position, "hh:mm:ss")
        }
    }

    Rectangle {
        id: progressIndicator
        color: Theme.themeColor
        anchors.left: parent.left
        y: root.height
        height: Constants.progressIndicatorHeight
        width: root.width * (player.position / player.duration)
    }
}
