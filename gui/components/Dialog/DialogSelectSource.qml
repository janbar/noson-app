/*
 * Copyright (C) 2016, 2017
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3


DialogBase {
    id: dialogSelectSource
    //: this is a title of a dialog to select source
    title: qsTr("Select source")
    standardButtons: Dialog.Close

    Label {
        id: sourceOutput
        anchors.left: parent.left
        anchors.right: parent.right
        wrapMode: Text.WordWrap
        color: "red"
        font.pointSize: units.fs("x-small")
        font.weight: Font.Normal
        visible: false // should only be visible when an error is made.
    }

    TextField {
        id: url
        text: inputStreamUrl
        font.pointSize: units.fs("medium")
        placeholderText: qsTr("Enter stream URL")
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
    }

    Connections {
        target: player.zonePlayer
        onJobFailed: {
            sourceOutput.text = qsTr("Playing failed.")
            sourceOutput.visible = true
        }
    }

    Button {
        id: buttonPlayStream
        height: units.gu(6)
        text: qsTr("Play stream")
        onClicked: {
            sourceOutput.visible = false // make sure its hidden now if there was an error last time
            if (url.text.length > 0) { // make sure something is acually inputed
                if (player.playStream(url.text, "")) {
                    inputStreamUrl = url.text
                }
                else {
                    sourceOutput.text = qsTr("Playing failed.")
                    sourceOutput.visible = true
                }
            }
            else {
                sourceOutput.visible = true
                sourceOutput.text = qsTr("Please type in an URL.")
            }
        }
    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        text: qsTr("Select the audio input.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        font.pointSize: units.fs("medium")
        font.weight: Font.Normal
    }

    ListModel {
        id: selectorModel
        ListElement { text: qsTr("Queue") }
        ListElement { text: qsTr("Play line IN") }
        ListElement { text: qsTr("Play TV") }
    }

    ComboBox {
        id: selector
        textRole: "text"
        model: selectorModel
        Layout.fillWidth: true
        font.pointSize: units.fs("large")
        currentIndex: 0
        Component.onCompleted: {
            popup.font.pointSize = font.pointSize;
        }
        onActivated: {
            switch(index) {
                case 0:
                    if (!player.playQueue(false))
                        popInfo.open(qsTr("Action can't be performed"))
                    else
                        dialogSelectSource.accept()
                    break;
                case 1:
                    if (!player.playLineIN())
                        popInfo.open(qsTr("Action can't be performed"))
                    else
                        dialogSelectSource.accept()
                    break;
                case 2:
                    if (!player.playDigitalIN())
                        popInfo.open(qsTr("Action can't be performed"))
                    else
                        dialogSelectSource.accept()
                    break;
                default:
                    break;
            }
        }
    }

}
