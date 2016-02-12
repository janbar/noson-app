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

import QtQuick 2.4
import Ubuntu.Components 1.3
import "Delegates"
import "Flickables"

MusicListView {
    id: renderingControlList
    anchors {
        fill: parent
    }
    model: player.renderingModel
    objectName: "renderingControlList"

    property color backgroundColor: styleMusic.mainView.backgroundColor
    property color labelColor: styleMusic.mainview.labelColor
    property bool isHeld: false

    signal finger(bool isHeld)

    delegate: SimpleListItem {
        id: renderingControlListItem
        multiselectable: false
        reorderable: false
        objectName: "renderingControlListItem" + index

        color: renderingControlList.backgroundColor

        column: Column {
            anchors {
                top: parent.top
            }

            Label {
                id: nameLabel
                fontSize: "small"
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
                MouseArea {
                    id: muteButton
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height
                    opacity: 1
                    width: height
                    onClicked: {
                        if (player.toggleMute(model.uuid))
                            player.renderingModel.setMute(index, !model.mute)
                        finger(isHeld)
                    }

                    Icon {
                        id: muteIcon
                        height: units.gu(3)
                        width: height
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        source: Qt.resolvedUrl("../graphics/volume_muted.png")
                        objectName: "muteShape"
                        opacity: model.mute ? 1.0 : 0.2
                    }
                }

                Slider {
                    id: volumeSlider
                    anchors.left: muteButton.right
                    anchors.rightMargin: units.gu(2)
                    anchors.right: gripButton.left
                    anchors.verticalCenter: parent.verticalCenter
                    live: true
                    minimumValue: 0
                    maximumValue: 100
                    objectName: "volumeSliderShape"
                    value: model.volume // load value at startup
                    opacity: 1.0

                    style: SoundSliderStyle {
                    }

                    Timer {
                        interval: 200
                        repeat: true
                        running: volumeSlider.pressed
                        onTriggered: {
                            if (player.setVolume(model.uuid, volumeSlider.value))
                                player.renderingModel.setVolume(index, volumeSlider.value)
                            finger(isHeld)
                        }
                    }

                    onPressedChanged: {
                        if (!pressed) {
                            if (player.setVolume(model.uuid, volumeSlider.value))
                                player.renderingModel.setVolume(index, volumeSlider.value)
                        }
                        else
                            finger(isHeld)
                    }
                }

                /* Grip button */
                MouseArea {
                    id: gripButton
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height
                    opacity: 1
                    width: units.gu(3)
                    onPressed: finger(isHeld)
                    onPressAndHold: {
                        isHeld = !isHeld
                        finger(isHeld)
                    }

                    Icon {
                        id: grip
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        source: Qt.resolvedUrl("../graphics/grip.svg")
                        width: units.gu(3)
                        height: width
                        opacity: renderingControlList.isHeld ? 1.0 : 0.3
                    }
                }
            }
        }
    }
}
