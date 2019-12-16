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

import QtQuick 2.2
import Sailfish.Silica 1.0
import "Delegates"
import "Flickables"

MusicListView {
    id: renderingControlList
    anchors.fill: parent
    clip: true

    model: player.renderingModel
    objectName: "renderingControlList"

    property color backgroundColor: styleMusic.view.backgroundColor
    property color foregroundColor: styleMusic.view.foregroundColor
    property color labelColor: styleMusic.view.labelColor
    property bool held: false

    signal finger(bool isHeld)

    delegate: SimpleListItem {
        id: renderingControlListItem
        objectName: "renderingControlListItem" + index

        color: renderingControlList.backgroundColor

        column: Column {
            anchors.top: parent.top
            width: parent.width

            Label {
                id: nameLabel
                font.pixelSize: units.fx("medium")
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
                    height: units.gu(5)
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
                    wheelEnabled: false // wheel cannot be handle here
                    stepSize: 2.5
                    live: true
                    from: 0
                    to: 100
                    objectName: "volumeSliderShape"
                    value: model.volume
                    enabled: !model.outputFixed
                    opacity: (model.outputFixed ? 0.2 : 1.0)

                    handleSize: (model.outputFixed ? 0 : units.gu(2))
                    handleColor: labelColor
                    handleBorderColor: handleColor
                    backgroundColor: styleMusic.playerControls.volumeBackgroundColor
                    foregroundColor: styleMusic.playerControls.volumeForegroundColor

                    onValueChanged: {
                        if (pressed)
                            setVolume.start();
                    }

                    onPressedChanged: finger(held)

                    Timer {
                        interval: 200
                        repeat: true
                        running: volumeSlider.pressed
                        onTriggered: finger(held)
                    }

                    Timer {
                        id: setVolume
                        interval: 200
                        onTriggered: {
                            if (!player.setVolume(model.uuid, volumeSlider.value)) {
                                customdebug("Set volume failed for zone " + model.uuid);
                            }
                            finger(held);
                        }
                    }
                }

                /* Grip button */
                Icon {
                    id: gripButton
                    anchors.right: parent.right
                    anchors.rightMargin: - units.gu(1)
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/images/grip.svg"
                    width: units.gu(5)
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
