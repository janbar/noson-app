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
        standardButtons: Dialog.Close
        contentSpacing: units.gu(1)
        edgeMargins: units.gu(0)

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
                    width: units.gu(10)

                }
                StyledSlider {
                    id: trebleSlider
                    live: true
                    from: -10
                    to: 10
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
                            if (trebleSlider.pressed) {
                                return
                            }

                            trebleSlider.value = player.treble
                        }
                    }

                    onPressedChanged: {
                        if (!pressed) {
                            value = player.treble
                        }
                    }

                    onMoved: {
                        var treble = Math.round(value)
                        if (treble === player.treble) {
                            return
                        }
                        player.setTreble(treble)
                    }
                }
            }

            Row {
                id: tickmarks
                width: parent.width
                spacing: 0
                Item {
                    id: c0
                    width: units.gu(11)
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
                    width: units.gu(10)
                }
                StyledSlider {
                    id: bassSlider
                    live: true
                    from: -10
                    to: 10
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
                            if (bassSlider.pressed) {
                                return
                            }

                            bassSlider.value = player.bass
                        }
                    }

                    onPressedChanged: {
                        if (!pressed) {
                            value = player.bass
                        }
                    }

                    onMoved: {
                        var bass = Math.round(value)
                        if (bass === player.bass) {
                            return
                        }
                        player.setBass(bass)
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
                    font.pointSize: units.fs("medium")
                    checked: player.nightmodeEnabled
                    onClicked: {
                        if (!player.toggleNightmode())
                            checked = player.nightmodeEnabled;
                    }
                    Connections {
                        target: player
                        onNightmodeEnabledChanged: nightmodeCheckbox.checked = player.nightmodeEnabled
                    }
                }
                MusicCheckBox {
                    id: loodness
                    anchors.right: parent.right
                    anchors.rightMargin: units.gu(1)
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: qsTr("Loudness")
                    textAlignment: Text.AlignHCenter
                    font.pointSize: units.fs("medium")
                    checked: false
                    enabled: false
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
                    font.pointSize: units.fs("medium")
                    checked: (player.currentProtocol === 1)
                    onClicked: {
                        if (checked) {
                            playTV.checked = false;
                            if (!player.playLineIN()) {
                                checked = false;
                                popInfo.open(qsTr("Action can't be performed"));
                            } else {
                                playTV.checked = false
                            }
                        } else if (!player.playQueue(false)) {
                            popInfo.open(qsTr("Action can't be performed"));
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
                    font.pointSize: units.fs("medium")
                    textAlignment: Text.AlignHCenter
                    checked: (player.currentProtocol === 5)
                    onClicked: {
                        if (checked) {
                            if (!player.playDigitalIN()) {
                                checked = false;
                                popInfo.open(qsTr("Action can't be performed"));
                            } else {
                                playIN.checked = false
                            }
                        } else if (!player.playQueue(false)) {
                            popInfo.open(qsTr("Action can't be performed"))
                        }
                    }
                    Connections {
                        target: player
                        onCurrentProtocolChanged: playTV.checked = (player.currentProtocol === 5)
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
