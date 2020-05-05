/*
 * Copyright (C) 2016-2019
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
    clip: true

    model: player.renderingModel
    objectName: "renderingControlList"

    property color backgroundColor: styleMusic.view.backgroundColor
    property color foregroundColor: styleMusic.view.foregroundColor
    property color labelColor: styleMusic.view.labelColor
    readonly property real rowHeight: units.gu(8)
    property bool volumePressed: false

    // try to reset the model on signal renderingControlChanged
    Connections {
        target: player
        onRenderingControlChanged: {
            refreshRendering.start()
        }
    }

    // delay the refresh while volume handle is pressed to avoid breaking of drag
    Timer {
        id: refreshRendering
        interval: 0
        onTriggered: {
            if (volumePressed) {
                interval = 100; // retry in 100ms
                restart();
            } else {
                player.refreshRendering();
                interval = 0;
            }
        }
    }

    delegate: SimpleListItem {
        id: renderingControlListItem
        objectName: "renderingControlListItem" + index

        color: renderingControlList.backgroundColor
        contentHeight: renderingControlList.rowHeight

        column: Column {
            id: column
            anchors.top: parent.top
            width: parent.width
            spacing: 0

            Label {
                id: nameLabel
                font.pixelSize: units.fx("small")
                text: model.name
                color: renderingControlList.labelColor
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
                maximumLineCount: 1
                verticalAlignment: Text.AlignVCenter
                anchors {
                    leftMargin: units.gu(3)
                    left: parent.left
                }
            }

            /* volume slider component */
            Item {
                id: controlerContainer
                anchors {
                    left: parent.left
                    leftMargin: units.gu(1.5)
                    right: parent.right
                }
                height: units.gu(5)
                width: parent.width

                /* Mute button */
                MusicIcon {
                    id: muteButton
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    height: units.gu(5)
                    width: height
                    source: model.mute ? "qrc:/sfos/images/icon-m-speaker-muted.svg" : "qrc:/sfos/images/icon-m-speaker.svg"
                    color: renderingControlList.foregroundColor
                    onClicked: {
                        player.toggleMute(model.uuid, function(result) {
                            if (result) {
                                player.renderingModel.setMute(index, !model.mute);
                            } else {
                                mainView.actionFailed();
                            }
                        });
                    }
                }

                StyledSlider {
                    id: volumeSlider
                    objectName: "volumeSliderShape"
                    anchors.leftMargin: units.gu(1.5)
                    anchors.left: muteButton.right
                    anchors.right: gripButton.left
                    anchors.verticalCenter: parent.verticalCenter
                    leftMargin: units.gu(2)
                    rightMargin: units.gu(2)
                    stepSize: 2.5
                    minimumValue: 0
                    maximumValue: 100
                    animateValue: true
                    enabled: !model.outputFixed
                    opacity: (model.outputFixed ? 0.2 : 1.0)
                    value: model.volume

                    onValueChanged: {
                        if (down)
                            setVolume.start();
                    }

                    onDownChanged: {
                        // report the handle is pressed to delay the model reset
                        renderingControlList.volumePressed = down;
                    }

                    Timer {
                        id: setVolume
                        interval: 200
                        property bool ready: true // false: delay the call
                        onTriggered: {
                            if (!ready) {
                                player.setVolumeForFake(model.uuid, volumeSlider.value);
                                restart();
                            } else {
                                ready = false;
                                player.setVolume(model.uuid, volumeSlider.value, function(result) {
                                    ready = true; // become ready on finished
                                    if (!result) {
                                        customdebug("Set volume failed for zone " + model.uuid);
                                    }
                                });
                            }
                        }
                    }
                }

                /* Grip button */
                MusicIcon {
                    id: gripButton
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/sfos/images/icon-m-grip.svg"
                    opacity: 0.1
                    height: units.gu(6)
                    width: height
                    color: renderingControlList.foregroundColor
                }
            }
        }
    }
}
