/*
 * Copyright (C) 2016
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
import "Delegates"
import "Flickables"

MusicListView {
    id: renderingControlList
    anchors.fill: parent
    clip: true

    model: player.renderingModel
    objectName: "renderingControlList"

    property color backgroundColor: styleMusic.mainView.backgroundColor
    property color foregroundColor: styleMusic.mainView.foregroundColor
    property color labelColor: styleMusic.mainView.labelColor
    property bool held: false

    signal finger(bool isHeld)

    delegate: SimpleListItem {
        id: renderingControlListItem
        objectName: "renderingControlListItem" + index

        color: renderingControlList.backgroundColor

        column: Column {
            anchors {
                top: parent.top
            }

            Label {
                id: nameLabel
                font.pointSize: units.fs("medium")
                font.weight: Font.Normal
                text: model.name
                color: renderingControlList.labelColor
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
                maximumLineCount: 1
                verticalAlignment: Text.AlignVCenter
                anchors {
                    leftMargin: units.gu(2)
                    left: parent.left
                }
            }

            /* volume slider component */
            Item {
                id: controlerContainer
                anchors {
                    left: parent.left
                    leftMargin: units.gu(2)
                    right: parent.right
                }
                height: units.gu(5)
                width: parent.width

                /* Mute button */
                Icon {
                    id: muteButton
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    height: units.gu(4)
                    width: height
                    source: model.mute ? "qrc:/images/audio-volume-muted.svg" : "qrc:/images/audio-volume.svg"
                    opacity: model.mute ? 1.0 : 0.6
                    color: renderingControlList.foregroundColor
                    onClicked: {
                        if (player.toggleMute(model.uuid))
                            player.renderingModel.setMute(index, !model.mute)
                        finger(held)
                    }
                }

                StyledSlider {
                    id: volumeSlider
                    anchors.leftMargin: units.gu(2)
                    anchors.left: muteButton.right
                    anchors.rightMargin: units.gu(2)
                    anchors.right: gripButton.left
                    anchors.verticalCenter: parent.verticalCenter
                    live: true
                    from: 0
                    to: 100
                    objectName: "volumeSliderShape"
                    value: model.volume // load value at startup
                    opacity: 1.0

                    handleSize: units.gu(2)
                    handleColor: labelColor
                    handleBorderColor: handleColor
                    backgroundColor: styleMusic.playerControls.volumeBackgroundColor
                    foregroundColor: styleMusic.playerControls.volumeForegroundColor

                    Timer {
                        interval: 200
                        repeat: true
                        running: volumeSlider.pressed
                        onTriggered: {
                            if (player.setVolume(model.uuid, volumeSlider.value))
                                player.renderingModel.setVolume(index, volumeSlider.value)
                            finger(held)
                        }
                    }

                    onPressedChanged: {
                        if (!pressed) {
                            if (player.setVolume(model.uuid, volumeSlider.value))
                                player.renderingModel.setVolume(index, volumeSlider.value)
                        }
                        else
                            finger(held)
                    }
                }

                /* Grip button */
                Icon {
                    id: gripButton
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/images/grip.svg"
                    width: units.gu(3)
                    height: width
                    opacity: renderingControlList.held ? 1.0 : 0.6
                    color: renderingControlList.foregroundColor
                    onPressed: finger(held)
                    onPressAndHold: {
                        held = !held
                        finger(held)
                    }
                }
            }
        }
    }
}
