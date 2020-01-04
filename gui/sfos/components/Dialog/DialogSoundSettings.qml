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

Item {
    id: soundSettings

    DialogBase {
        id: dialog

        //: this is a title of a dialog to configure sound settings
        title: qsTr("Sound settings")
        contentSpacing: units.gu(1)

        cancelText: qsTr("Close")
        acceptText: ""
        canAccept: false

        StyledSlider {
            id: trebleSlider
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin + units.gu(5)
                rightMargin: Theme.horizontalPageMargin + units.gu(5)
            }
            
            minimumValue: -10
            maximumValue: 10
            label: qsTr("Treble")
            stepSize: 1.0

            value: player.treble // load value at startup

            Connections {
                target: player
                onTrebleChanged:{
                    if (!trebleSlider.down) {
                        trebleSlider.value = player.treble;
                    }
                }
            }

            onValueChanged: {
                setTreble.start();
            }

            Timer {
                id: setTreble
                interval: 200
                property bool ready: true // false: delay the call
                onTriggered: {
                    if (!ready) {
                        restart();
                    } else {
                        ready = false;
                        player.setTreble(Math.round(trebleSlider.value), function(result) {
                            ready = true;
                            if (!result) {
                                customdebug("Set treble failed");
                            }
                        });
                    }
                }
            }
        }

        StyledSlider {
            id: bassSlider
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin + units.gu(5)
                rightMargin: Theme.horizontalPageMargin + units.gu(5)
            }
            
            minimumValue: -10
            maximumValue: 10
            label: qsTr("Bass")
            stepSize: 1.0

            value: player.bass // load value at startup

            Connections {
                target: player
                onBassChanged:{
                    if (!bassSlider.down) {
                        bassSlider.value = player.bass;
                    }
                }
            }

            onValueChanged: {
                setBass.start();
            }

            Timer {
                id: setBass
                interval: 200
                property bool ready: true // false: delay the call
                onTriggered: {
                    if (!ready) {
                        restart();
                    } else {
                        ready = false;
                        player.setBass(Math.round(bassSlider.value), function(result) {
                            ready = true;
                            if (!result) {
                                customdebug("Set bass failed");
                            }
                        });
                    }
                }
            }
        }

        Row {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }

            spacing: units.gu(1)

            Column {
                id: modes
                width: (parent.width - parent.spacing) / 2
                spacing: units.gu(1)
                MusicCheckBox {
                    id: nightMode
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: qsTr("Night mode")
                    checked: player.nightmodeEnabled
                    onClicked: {
                        player.toggleNightmode(function(result) {
                            if (!result) {
                                checked = player.nightmodeEnabled;
                                mainView.actionFailed();
                            }
                        });
                    }
                    Connections {
                        target: player
                        onNightmodeEnabledChanged: nightMode.checked = player.nightmodeEnabled
                    }
                }
                MusicCheckBox {
                    id: loudness
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: qsTr("Loudness")
                    checked: player.loudnessEnabled
                    onClicked: {
                        player.toggleLoudness(function(result) {
                            if (!result) {
                                checked = player.loudnessEnabled;
                                mainView.actionFailed();
                            }
                        });
                    }
                    Connections {
                        target: player
                        onLoudnessEnabledChanged: loudness.checked = player.loudnessEnabled
                    }
                }
            }

            Column {
                id: inputs
                width: (parent.width - parent.spacing) / 2
                spacing: units.gu(1)
                MusicCheckBox {
                    id: playIN
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: qsTr("Line IN")
                    checked: (player.currentProtocol === 1)
                    onClicked: {
                        if (checked) {
                            player.playLineIN(function(result) {
                                if (!result) {
                                    checked = false;
                                    mainView.actionFailed();
                                } else {
                                    playTV.checked = false
                                    playPulse.checked = false
                                }
                            });
                        } else {
                            player.playQueue(false, mainView.actionFinished);                        }
                    }
                    Connections {
                        target: player
                        onCurrentProtocolChanged: playIN.checked = (player.currentProtocol === 1)
                    }
                }
                MusicCheckBox {
                    id: playTV
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: qsTr("TV")
                    checked: (player.currentProtocol === 5)
                    onClicked: {
                        if (checked) {
                            player.playDigitalIN(function(result) {
                                if (!result) {
                                    checked = false;
                                    mainView.actionFailed();
                                } else {
                                    playIN.checked = false
                                    playPulse.checked = false
                                }
                            });
                        } else {
                            player.playQueue(false, mainView.actionFinished);
                        }
                    }
                    Connections {
                        target: player
                        onCurrentProtocolChanged: playTV.checked = (player.currentProtocol === 5)
                    }
                }
                MusicCheckBox {
                    id: playPulse
                    //visible: Sonos.havePulseAudio()
                    visible: false // No usage on Sailfish
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: "PulseAudio" // not translated
                    checked: (player.isPulseStream())
                    onClicked: {
                        if (checked) {
                            player.playPulse(function(result) {
                                if (!result) {
                                    checked = false;
                                    mainView.actionFailed();
                                } else {
                                    playIN.checked = false
                                    playTV.checked = false
                                }
                            });
                        } else {
                            player.playQueue(false, mainView.actionFinished);
                        }
                    }
                    Connections {
                        target: player
                        onCurrentMetaSourceChanged: playPulse.checked = (player.isPulseStream())
                    }
                }
            }
        }

        onOpened: {
            switch (player.currentProtocol) {
            case 1:
                playIN.checked = true;
                playTV.checked = false;
                break;
            case 5:
                playIN.checked = false;
                playTV.checked = true;
                break;
            default:
                playIN.checked = false;
                playTV.checked = false;
            }
        }
    }

    function open() {
        return dialog.open();
    }
}
