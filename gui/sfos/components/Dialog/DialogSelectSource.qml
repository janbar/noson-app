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

import QtQuick 2.2
import Sailfish.Silica 1.0
import QtQuick.Layouts 1.1
import NosonApp 1.0


DialogBase {
    id: dialog
    //: this is a title of a dialog to select source
    title: qsTr("Select source")

    property bool showSelector: false

    acceptText: qsTr("Close")

    Label {
        id: sourceOutput
        anchors.left: parent.left
        anchors.right: parent.right
        wrapMode: Text.WordWrap
        color: "red"
        font.pixelSize: units.fx("x-small")
        font.weight: Font.Normal
        visible: false // should only be visible when an error is made.
    }

    TextField {
        id: url
        text: inputStreamUrl
        width: parent.width
        placeholderText: qsTr("Enter stream URL")
    }

    Connections {
        target: player
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
        visible: showSelector
        anchors.left: parent.left
        anchors.right: parent.right
        text: qsTr("Select the audio input.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        font.weight: Font.Normal
    }

    ListModel {
        id: selectorModel
        ListElement { text: qsTr("Queue") }
        ListElement { text: qsTr("Play line IN") }
        ListElement { text: qsTr("Play TV") }
        ListElement { text: qsTr("Play PulseAudio") }
    }

    ComboBox {
        id: selector
        visible: showSelector
        menu: ContextMenu {
                MenuItem {
                    text: qsTr("Queue")
                    onClicked: {
                        if (!player.playQueue(false))
                            popInfo.open(qsTr("Action can't be performed"))
                        else
                            dialog.accept()
                    }
                }
                MenuItem {
                    text: qsTr("Play line IN")
                    onClicked: {
                        if (!player.playLineIN())
                            popInfo.open(qsTr("Action can't be performed"))
                        else
                            dialog.accept()
                    }
                }
                MenuItem {
                    text: qsTr("Play TV")
                    onClicked: {
                        if (!player.playDigitalIN())
                            popInfo.open(qsTr("Action can't be performed"))
                        else
                            dialog.accept()
                    }
                }
                MenuItem {
                    text: qsTr("Play PulseAudio")
                    visible: Sonos.havePulseAudio()
                    onClicked: {
                        if (!player.playPulse())
                            popInfo.open(qsTr("Action can't be performed"))
                        else
                            dialog.accept()
                    }
                }
        }
        
        Layout.fillWidth: true
        currentIndex: 0
    }

    onOpened: {
        if (player.currentMetaSource === "") {
            showSelector = true;
            switch (player.currentProtocol) {
                case 1:
                    selector.currentIndex = 1;
                    break;
                case 5:
                    selector.currentIndex = 2;
                    break;
                default:
                    selector.currentIndex = 0;
            }
        }
    }

}
