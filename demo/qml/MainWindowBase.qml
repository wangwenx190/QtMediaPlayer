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
import org.wangwenx190.QtMediaPlayer
import wangwenx190.QtMediaPlayer.Demo

FramelessWindow {
    property alias playerItem: player

    id: window
    width: Constants.defaultWindowWidth
    height: Constants.defaultWindowHeight
    title: (player.playbackState === QtMediaPlayer.Stopped) || (player.fileName.length < 1)
                 ? Application.displayName : player.fileName
    color: Theme.windowBackgroundColor
    onVisibilityChanged: {
        if ((visibility === Window.Minimized)
                && player.isPlayingVideo() && Settings.pauseWhenMinimized) {
            player.pause();
        }
    }
    Component.onCompleted: {
        if (Settings.restoreLastWindowGeometry) {
            x = Settings.x;
            y = Settings.y;
            width = Settings.width;
            height = Settings.height;
        } else {
            moveToCenter();
        }
        // Default is false, so the window won't be visible unless
        // we explicitly set it to true.
        visible = true;
    }
    Component.onDestruction: {
        Settings.x = x;
        Settings.y = y;
        Settings.width = width;
        Settings.height = height;
        Settings.mute = player.mute;
        Settings.volume = player.volume;
        Settings.hardwareDecoding = player.hardwareDecoding;
        Settings.logLevel = player.logLevel;
    }

    Item {
        id: windowStateItem
        states: [
            State {
                name: "hide_titleBar"

                PropertyChanges {
                    target: titleBar
                    y: -Constants.titleBarHeight
                }
            },
            State {
                name: ""

                PropertyChanges {
                    target: titleBar
                    y: 0
                }
            }
        ]
        transitions: [
            Transition {
                from: ""
                to: "hide_titleBar"

                PropertyAnimation {
                    target: titleBar
                    property: "y"
                    duration: Constants.titleBarHideDuration
                }
            },
            Transition {
                from: "hide_titleBar"
                to: ""

                PropertyAnimation {
                    target: titleBar
                    property: "y"
                    duration: Constants.titleBarHideDuration
                }
            }
        ]
    }

    Player {
        objectName: "mediaPlayer"
        id: player
        anchors.fill: parent
        visible: playbackState !== QtMediaPlayer.Stopped
        logLevel: QtMediaPlayer.Debug
        mute: Settings.mute
        volume: Settings.volume
        hardwareDecoding: Settings.hardwareDecoding
        snapshotFormat: Settings.snapshotFormat
        snapshotDirectory: Settings.snapshotDirectory
        windowed: window.visibility === Window.Windowed
        onLoaded: titleBar.state = "hide_background"
        onStopped: {
            if (window.visibility === Window.FullScreen) {
                window.toggleFullScreen();
            }
            windowStateItem.state = "";
            titleBar.state = "";
            if (window.visibility === Window.Windowed) {
                window.width = Constants.defaultWindowWidth;
                window.height = Constants.defaultWindowHeight;
                window.moveToCenter();
            }
        }
        onRecommendedWindowSizeChanged: {
            if (window.visibility === Window.Windowed) {
                let windowSize = recommendedWindowSize;
                if ((windowSize.width > 9) && (windowSize.height > 9)) {
                    window.width = windowSize.width;
                    window.height = windowSize.height;
                }
            }
        }
        onRecommendedWindowPositionChanged: {
            if (window.visibility === Window.Windowed) {
                let windowPos = recommendedWindowPosition;
                if ((windowPos.x >= 0) && (windowPos.y >= 0)) {
                    window.x = windowPos.x;
                    window.y = windowPos.y;
                }
            }
        }
        onRequestToggleFullScreen: window.toggleFullScreen()
        onRequestHideTitleBar: windowStateItem.state = "hide_titleBar"
        onRequestShowTitleBar: windowStateItem.state = ""
        onRequestMoveWindow: {
            if (window.visibility === Window.Windowed) {
                window.startSystemMove2();
            }
        }
    }

    Item {
        anchors.centerIn: parent
        width: 100
        height: width
        visible: player.playbackState === QtMediaPlayer.Stopped

        Image {
            anchors.fill: parent
            // Set "mipmap" to true if the source size is larger than the display size.
            mipmap: true
            source: Theme.darkModeEnabled ? "qrc:/images/light/windows-media-player.svg" : "qrc:/images/dark/windows-media-player.svg"
        }
    }

    DropArea {
        anchors.fill: parent
        // ### FIXME: why not working?
        //keys: [ player.videoFileMimeTypes, player.audioFileMimeTypes ]
        onDropped: {
            if (drop.hasUrls) {
                if ((drop.proposedAction === Qt.MoveAction) || (drop.proposedAction === Qt.CopyAction)) {
                    player.source = drop.urls[0];
                    drop.acceptProposedAction();
                    return;
                }
            }
            drop.accepted = false;
        }
    }

    MouseArea {
        anchors {
            top: titleBar.bottom
            bottom: parent.bottom
            bottomMargin: window.visibility === Window.Windowed ? Constants.resizeBorderThickness : 0
            left: parent.left
            leftMargin: window.visibility === Window.Windowed ? Constants.resizeBorderThickness : 0
            right: parent.right
            rightMargin: window.visibility === Window.Windowed ? Constants.resizeBorderThickness : 0
        }
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        enabled: player.playbackState === QtMediaPlayer.Stopped
        onPositionChanged: {
            if (containsPress) {
                if (window.visibility === Window.Windowed) {
                    window.startSystemMove2();
                }
            }
        }
        onClicked: openMediaDialog.open()
    }

    TitleBar {
        id: titleBar
        anchors {
            left: parent.left
            right: parent.right
        }
        y: 0
        text: window.title
        maximized: window.visibility === Window.Maximized
        fullscreened: window.visibility === Window.FullScreen
        onMinimizeButtonClicked: {
            if (window.visibility !== Window.FullScreen) {
                window.showMinimized2();
            }
        }
        onMaximizeButtonClicked: {
            if (window.visibility !== Window.FullScreen) {
                window.toggleMaximized();
            }
        }
        onCloseButtonClicked: window.close()

        MouseArea {
            anchors {
                top: parent.top
                topMargin: window.visibility === Window.Windowed ? Constants.resizeBorderThickness : 0
                bottom: parent.bottom
                left: parent.left
                leftMargin: window.visibility === Window.Windowed ? Constants.resizeBorderThickness : 0
                right: parent.right
                rightMargin: parent.height * 1.5 * 3.0
            }
            hoverEnabled: true
            onPositionChanged: {
                if (containsPress) {
                    if (window.visibility !== Window.FullScreen) {
                        window.startSystemMove2();
                    }
                }
            }
            onDoubleClicked: {
                if (window.visibility !== Window.FullScreen) {
                    window.toggleMaximized();
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.MiddleButton
        onWheel: {
            if (window.visibility === Window.Windowed) {
                let angleY = wheel.angleDelta.y;
                if (angleY !== 0) {
                    if (angleY > 0) {
                        window.zoomIn(0.004);
                    } else {
                        window.zoomOut(0.004);
                    }
                }
            }
        }
    }

    OpenMediaDialog {
        id: openMediaDialog
        anchors.centerIn: parent
        width: parent.width * 0.6
        nameFilters: [
            qsTr("Video files (%1)").arg(player.videoFileSuffixes.join(' ')),
            qsTr("Audio files (%1)").arg(player.audioFileSuffixes.join(' ')),
            qsTr("All files (*)")
        ]
        file: Settings.file
        dir: Settings.dir
        onAccepted: {
            if (selectedUrl !== "") {
                Settings.file = selectedUrl;
                Settings.dir = dir;
                player.source = selectedUrl;
            }
        }
    }

    Shortcut {
        sequence: StandardKey.Open
        onActivated: openMediaDialog.open()
    }

    Shortcut {
        // "Alt+Enter" won't work, currently don't know why.
        sequences: [ StandardKey.FullScreen, "Alt+Return" ]
        onActivated: {
            if (player.playbackState !== QtMediaPlayer.Stopped) {
                window.toggleFullScreen();
            }
        }
    }

    Shortcut {
        sequences: [ StandardKey.Close, StandardKey.Quit, StandardKey.Cancel ]
        onActivated: {
            if (window.visibility === Window.FullScreen) {
                window.toggleFullScreen();
            } else {
                window.close();
            }
        }
    }

    Shortcut {
        sequence: "Space"
        onActivated: player.togglePlayPause()
    }

    Shortcut {
        sequence: "Left"
        onActivated: player.seekBackward()
    }

    Shortcut {
        sequence: "Right"
        onActivated: player.seekForward()
    }

    Shortcut {
        sequence: "Up"
        onActivated: player.volumeUp()
    }

    Shortcut {
        sequence: "Down"
        onActivated: player.volumeDown()
    }
}
