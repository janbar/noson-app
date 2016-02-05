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
import Ubuntu.Components 1.2
import Ubuntu.Components.Popups 1.0
import NosonApp 1.0
import "../Delegates"
import "../"

Item {
    id: songInfo
    property var model: null
    property bool showActions: false

    Component {
        id: dialogComponent
        Dialog {
            id: dialog

            Component.onCompleted: {
                if (songInfo.model) {
                    card.coverSources = [{art: songInfo.model.art}];
                    card.primaryText = songInfo.model.title !== "" ? songInfo.model.title : i18n.tr("Unknown Album");
                    card.secondaryText = songInfo.model.author !== "" ? songInfo.model.author : i18n.tr("Unknown Artist");
                    card.tertiaryLabelVisible = songInfo.model.album.length !== "";
                    card.tertiaryText = i18n.tr("%1 - track #%2").arg(songInfo.model.album).arg(songInfo.model.albumTrackNo);
                    card.actionsVisible = showActions
                }
            }

            Item {
                id: card
                height: cardColumn.childrenRect.height + actions.height

                property alias coverSources: coverGrid.covers
                property alias primaryText: primaryLabel.text
                property alias secondaryText: secondaryLabel.text
                property alias tertiaryText: tertiaryLabel.text
                property alias tertiaryLabelVisible: tertiaryLabel.visible
                property alias actionsVisible: actions.visible

                signal clicked(var mouse)
                signal pressAndHold(var mouse)

                /* Animations */
                Behavior on height {
                    UbuntuNumberAnimation {

                    }
                }

                Behavior on width {
                    UbuntuNumberAnimation {

                    }
                }

                Behavior on x {
                    UbuntuNumberAnimation {

                    }
                }

                Behavior on y {
                    UbuntuNumberAnimation {

                    }
                }

                /* Background for card */
                Rectangle {
                    id: bg
                    anchors {
                        fill: cardColumn
                    }
                    color: "transparent"
                }

                /* Column containing image and labels */
                Column {
                    id: cardColumn
                    anchors {
                        top: parent.top
                    }
                    width: parent.width
                    spacing: units.gu(0.5)

                    CoverGrid {
                        id: coverGrid
                        size: parent.width
                    }

                    Item {
                        height: units.gu(1)
                        width: units.gu(1)
                    }

                    Label {
                        id: primaryLabel
                        anchors {
                            left: parent.left
                            leftMargin: units.gu(1)
                            right: parent.right
                            rightMargin: units.gu(1)
                        }
                        color: styleMusic.popover.labelColor
                        elide: Text.ElideRight
                        fontSize: "large"
                        opacity: 1.0
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: secondaryLabel
                        anchors {
                            left: parent.left
                            leftMargin: units.gu(1)
                            right: parent.right
                            rightMargin: units.gu(1)
                        }
                        color: styleMusic.popover.labelColor
                        elide: Text.ElideRight
                        fontSize: "small"
                        opacity: 0.4
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: tertiaryLabel
                        anchors {
                            left: parent.left
                            leftMargin: units.gu(1)
                            right: parent.right
                            rightMargin: units.gu(1)
                        }
                        color: styleMusic.popover.labelColor
                        elide: Text.ElideRight
                        fontSize: "small"
                        opacity: 0.4
                        wrapMode: Text.WordWrap
                    }

                    Item {
                        height: units.gu(1.5)
                        width: units.gu(1)
                    }
                }

                /* Overlay for when card is pressed */
                Rectangle {
                    id: overlay
                    anchors {
                        fill: bg
                    }
                    color: styleMusic.popover.foregroundColor
                    opacity: 0

                    Behavior on opacity {
                        UbuntuNumberAnimation {

                        }
                    }
                }

                /* Capture mouse events */
                MouseArea {
                    anchors {
                        fill: bg
                    }
                    onClicked: card.clicked(mouse)
                    onPressAndHold: card.pressAndHold(mouse)
                    onPressedChanged: overlay.opacity = pressed ? 0.3 : 0
                }

                onClicked: {
                    timer.stop();
                    PopupUtils.close(dialog);
                }

                /* Actions area */
                Rectangle {
                    id: actions
                    anchors {
                        top: cardColumn.bottom
                        left: parent.left
                        right: parent.right
                    }
                    visible: false
                    height: visible ? units.gu(8) : units.gu(0)
                    color: "transparent"

                    /* Play button */
                    Icon {
                        id: playerControlsPlayButton
                       anchors {
                           horizontalCenter: parent.horizontalCenter
                           verticalCenter: parent.verticalCenter
                        }
                        color: styleMusic.popover.labelColor
                        height: units.gu(6)
                        name: "media-playback-start"
                        objectName: "playShape"
                        width: height
                    }

                    /* Mouse area for the play button (ontop of the jump to now playing) */
                    MouseArea {
                        anchors {
                            top: parent.top
                            bottom: parent.bottom
                            horizontalCenter: playerControlsPlayButton.horizontalCenter
                        }
                        onClicked: {
                            timer.stop();
                            PopupUtils.close(dialog);
                            trackClicked(songInfo.model); // play track
                        }
                        width: playerControlsPlayButton.width + units.gu(2)

                        Rectangle {
                            anchors {
                                fill: parent
                            }
                            color: styleMusic.popover.backgroundColor
                            opacity: parent.pressed ? 0.1 : 0

                            Behavior on opacity {
                                UbuntuNumberAnimation {
                                    duration: UbuntuAnimation.FastDuration
                                }
                            }
                        }
                    }
                }

            } // Item

            Behavior on opacity {
                NumberAnimation { duration: 500 }
            }

            Timer {
                id: timer
                interval: 10000

                Component.onCompleted: start()

                onTriggered: {
                    opacity = 0
                    closingDialogTimer.start()
                }
            }

            Timer {
                id: closingDialogTimer
                interval: 500

                onTriggered: {
                    visible = false
                    PopupUtils.close(dialog)
                }
            }
        }
    }

    function open(model, showActions) {
        songInfo.model = model;
        songInfo.showActions = showActions;
        return PopupUtils.open(dialogComponent);
    }
}
