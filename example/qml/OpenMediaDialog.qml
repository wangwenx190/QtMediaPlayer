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
import QtQuick.Dialogs
import QtQuick.Layouts

Dialog {
    property alias selectedUrl: textField.text
    property alias nameFilters: fileDialog.nameFilters
    property alias dir: fileDialog.currentFolder
    property alias file: fileDialog.currentFile

    id: dialog
    title: qsTr("Open Media")
    modal: true
    closePolicy: Popup.CloseOnEscape
    standardButtons: Dialog.Ok | Dialog.Cancel

    contentItem: Item {
        width: 200
        height: 50

        RowLayout {
            anchors.fill: parent
            spacing: 10

            Label {
                text: qsTr("URL")
                height: parent.height
                verticalAlignment: Text.AlignVCenter
            }

            TextField {
                id: textField
                placeholderText: qsTr("Please input a valid local file path or network URL")
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Browse")
                onClicked: fileDialog.open()
            }
        }
    }

    FileDialog {
        id: fileDialog
        options: FileDialog.ReadOnly
        onAccepted: textField.text = selectedFile
    }
}
