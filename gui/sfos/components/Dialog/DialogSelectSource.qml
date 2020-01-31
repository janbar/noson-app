/*
 * Copyright (C) 2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
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
import "../"

DialogBase {
    id: dialog
    //: this is a title of a dialog to select source
    title: qsTr("Select source")

    property bool showSelector: false

    cancelText: qsTr("Close")
    acceptText: ""
    canAccept: false

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
        label: qsTr("Select the audio input:")
        anchors {
            left: parent.left
            right: parent.right
        }
        menu: ContextMenu {
                MenuItem {
                    text: qsTr("Queue")
                    onClicked: {
                        player.playQueue(false, function(result) {
                            if (result) {
                                dialog.close();
                            } else {
                                mainView.actionFailed();
                            }
                        });
                    }
                }
                MenuItem {
                    text: qsTr("Play line IN")
                    onClicked: {
                        player.playLineIN(function(result) {
                            if (result) {
                                dialog.close()
                            } else {
                                mainView.actionFailed();
                            }
                        });
                    }
                }
                MenuItem {
                    text: qsTr("Play TV")
                    onClicked: {
                        player.playDigitalIN(function(result) {
                            if (result) {
                                dialog.close()
                            } else {
                                mainView.actionFailed();
                            }
                        });
                    }
                }
                MenuItem {
                    text: qsTr("Play PulseAudio")
                    //visible: Sonos.havePulseAudio()
                    visible: false // No usage on Sailfish
                    onClicked: {
                        player.playPulse(function(result) {
                            if (result) {
                                dialog.close()
                            } else {
                                mainView.actionFailed();
                            }
                        });
                    }
                }
        }

        currentIndex: 0
    }

    Item {
        id: separator
        visible: showSelector
        width: parent.width
        height: units.gu(3)
    }

    Label {
        id: sourceOutput
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        wrapMode: Text.WordWrap
        color: "red"
        font.pixelSize: units.fx("x-small")
        font.weight: Font.Normal
        visible: false // should only be visible when an error is made.
    }

    TextField {
        id: url
        anchors.left: parent.left
        anchors.right: parent.right
        text: inputStreamUrl
        placeholderText: qsTr("Enter stream URL")
    }

    function actionFailed() {
        sourceOutput.text = qsTr("Playing failed.")
        sourceOutput.visible = true
    }

    MusicIcon {
        id: buttonPlayStream
        anchors.horizontalCenter: parent.horizontalCenter
        label.text: qsTr("Play stream")
        source: "image://theme/icon-m-play"
        height: units.gu(6)
        onClicked: {
            sourceOutput.visible = false // make sure its hidden now if there was an error last time
            if (url.text.length > 0) { // make sure something is acually inputed
                player.playStream(url.text, "", function(result) {
                    if (result) {
                        inputStreamUrl = url.text;
                    } else {
                        sourceOutput.text = qsTr("Playing failed.");
                        sourceOutput.visible = true;
                    }
                });
            }
            else {
                sourceOutput.visible = true
                sourceOutput.text = qsTr("Please type in an URL.")
            }
        }
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
        } else {
           showSelector = false;
        }
    }

}
