/*
 * Copyright (C) 2018
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
import NosonApp 1.0
import "../"

Item {
    id: soundSettings

    DialogBase {
        id: dialog

        //: this is a title of a dialog to configure sound settings
        title: qsTr("Sound settings")
        contentSpacing: units.gu(1)
        edgeMargins: units.gu(0)

        footer: Row {
            leftPadding: units.gu(1)
            rightPadding: units.gu(1)
            bottomPadding: units.gu(1)
            spacing: units.gu(1)
            layoutDirection: Qt.RightToLeft

            Button {
                flat: true
                text: qsTr("Close")
                onClicked: dialog.reject()
            }
        }

        Column {
            spacing: 0
            Row {
                width: parent.width
                spacing: units.gu(1)

                Label {
                    id: trebleLabel
                    text: qsTr("Treble")
                    color: styleMusic.dialog.labelColor
                    font.pointSize: units.fs("medium")
                    verticalAlignment: Text.AlignVCenter
                    height: trebleSlider.height
                    width: units.gu(12)

                }
                StyledSlider {
                    id: trebleSlider
                    live: true
                    from: -10
                    to: 10
                    wheelEnabled: true
                    orientation: Qt.Horizontal
                    handleSize: units.gu(2)
                    handleColor: styleMusic.playerControls.volumeHandleColor
                    handleColorPressed: styleMusic.playerControls.backgroundColor
                    handleBorderColor: handleColor
                    backgroundColor: styleMusic.playerControls.volumeBackgroundColor
                    foregroundColor: styleMusic.playerControls.volumeForegroundColor
                    size: parent.width - trebleLabel.width - parent.spacing - units.gu(1)
                    stepSize: 1.0

                    value: player.treble // load value at startup

                    Connections {
                        target: player
                        onTrebleChanged:{
                            if (!trebleSlider.pressed) {
                                trebleSlider.value = player.treble;
                            }
                        }
                    }

                    onMoved: {
                        setTreble.start();
                    }

                    Timer {
                        id: setTreble
                        interval: 300
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
            }

            Row {
                id: tickmarks
                width: parent.width
                spacing: 0
                Item {
                    id: c0
                    width: units.gu(13)
                    height: units.gu(1)
                }
                Label {
                    width: (parent.width - c0.width - zero.width - units.gu(1)) / 2
                    text: "-10"
                    font.pointSize: units.fs("x-small")
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    color: styleMusic.playerControls.volumeForegroundColor
                }
                Label {
                    id: zero
                    text: "0"
                    font.pointSize: units.fs("x-small")
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: styleMusic.playerControls.volumeForegroundColor
                }
                Label {
                    width: (parent.width - c0.width - zero.width - units.gu(1)) / 2
                    text: "+10"
                    font.pointSize: units.fs("x-small")
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignRight
                    color: styleMusic.playerControls.volumeForegroundColor
                }
            }

            Row {
                width: parent.width
                spacing: units.gu(1)

                Label {
                    id: bassLabel
                    text: qsTr("Bass")
                    color: styleMusic.dialog.labelColor
                    font.pointSize: units.fs("medium")
                    verticalAlignment: Text.AlignVCenter
                    height: bassSlider.height
                    width: units.gu(12)
                }
                StyledSlider {
                    id: bassSlider
                    live: true
                    from: -10
                    to: 10
                    wheelEnabled: true
                    orientation: Qt.Horizontal
                    handleSize: units.gu(2)
                    handleColor: styleMusic.playerControls.volumeHandleColor
                    handleColorPressed: styleMusic.playerControls.backgroundColor
                    handleBorderColor: handleColor
                    backgroundColor: styleMusic.playerControls.volumeBackgroundColor
                    foregroundColor: styleMusic.playerControls.volumeForegroundColor
                    size: parent.width - trebleLabel.width - parent.spacing - units.gu(1)
                    stepSize: 1.0

                    value: player.bass // load value at startup

                    Connections {
                        target: player
                        onBassChanged:{
                            if (!bassSlider.pressed) {
                                bassSlider.value = player.bass;
                            }
                        }
                    }

                    onMoved: {
                        setBass.start();
                    }

                    Timer {
                        id: setBass
                        interval: 300
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
            }

            Row {
                width: parent.width
                spacing: units.gu(1)

                Label {
                    id: subGainLabel
                    text: qsTr("Subwoofer")
                    color: styleMusic.dialog.labelColor
                    font.pointSize: units.fs("medium")
                    verticalAlignment: Text.AlignVCenter
                    height: subGainSlider.height
                    width: units.gu(12)
                }
                StyledSlider {
                    id: subGainSlider
                    live: true
                    from: -10
                    to: 10
                    wheelEnabled: true
                    orientation: Qt.Horizontal
                    handleSize: units.gu(2)
                    handleColor: styleMusic.playerControls.volumeHandleColor
                    handleColorPressed: styleMusic.playerControls.backgroundColor
                    handleBorderColor: handleColor
                    backgroundColor: styleMusic.playerControls.volumeBackgroundColor
                    foregroundColor: styleMusic.playerControls.volumeForegroundColor
                    size: parent.width - trebleLabel.width - parent.spacing - units.gu(1)
                    stepSize: 1.0

                    value: player.subGain // load value at startup

                    Connections {
                        target: player
                        onSubGainChanged:{
                            if (!subGainSlider.pressed) {
                                subGainSlider.value = player.subGain;
                            }
                        }
                    }

                    onMoved: {
                        setSubGain.start();
                    }

                    Timer {
                        id: setSubGain
                        interval: 300
                        property bool ready: true // false: delay the call
                        onTriggered: {
                            if (!ready) {
                                restart();
                            } else {
                                ready = false;
                                player.setSubGain(Math.round(subGainSlider.value), function(result) {
                                    ready = true;
                                    if (!result) {
                                        customdebug("Set subGain failed");
                                    }
                                });
                            }
                        }
                    }
                }
            }
        }

        Row {
            width: parent.width
            spacing: units.gu(1)

            Column {
                id: modes
                width: (parent.width - parent.spacing) / 2
                spacing: 0
                MusicCheckBox {
                    id: nightMode
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: qsTr("Night mode")
                    textAlignment: Text.AlignHCenter
                    font.pointSize: units.fs("small")
                    checked: player.nightmodeEnabled
                    onClicked: {
                        player.toggleNightmode(function(result) {
                            if (!result) {
                                nightMode.checked = player.nightmodeEnabled;
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
                    textAlignment: Text.AlignHCenter
                    font.pointSize: units.fs("small")
                    checked: player.loudnessEnabled
                    onClicked: {
                        player.toggleLoudness(function(result) {
                            if (!result) {
                                loudness.checked = player.loudnessEnabled;
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
                spacing: 0
                MusicCheckBox {
                    id: playIN
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: qsTr("Line IN")
                    textAlignment: Text.AlignHCenter
                    font.pointSize: units.fs("small")
                    checked: (player.currentProtocol === 1)
                    onClicked: {
                        if (checked) {
                            player.playLineIN(function(result) {
                                if (!result) {
                                    playIN.checked = false;
                                    mainView.actionFailed();
                                } else {
                                    playTV.checked = false
                                    playPulse.checked = false
                                }
                            });
                        } else {
                            player.playQueue(false, mainView.actionFinished);
                        }
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
                    font.pointSize: units.fs("small")
                    textAlignment: Text.AlignHCenter
                    checked: (player.currentProtocol === 5)
                    onClicked: {
                        if (checked) {
                            player.playDigitalIN(function(result) {
                                if (!result) {
                                    playTV.checked = false;
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
                    visible: Sonos.havePulseAudio()
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: "PulseAudio" // not translated
                    textAlignment: Text.AlignHCenter
                    font.pointSize: units.fs("small")
                    checked: (player.isPulseStream())
                    onClicked: {
                        if (checked) {
                            player.playPulse(function(result) {
                                if (!result) {
                                    playPulse.checked = false;
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
