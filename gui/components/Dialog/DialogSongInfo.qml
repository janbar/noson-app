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

import QtQuick 2.9
import QtQuick.Controls 2.3
import NosonApp 1.0
import "../Delegates"
import "../"

Item {
    id: songInfo
    property var model: null
    property bool actionsVisible: false

    DialogBase {
        id: dialog
        standardButtons: Dialog.Close

        width: mainView.minimumWidth - units.gu(2)

        onOpened: {
            timer.restart();
            if (songInfo.model) {
                card.coverSources = [{art: makeCoverSource(songInfo.model.art, songInfo.model.author, songInfo.model.album)}];
                card.primaryText = songInfo.model.title !== "" ? songInfo.model.title : qsTr("Unknown Album");
                card.secondaryText = songInfo.model.author !== "" ? songInfo.model.author : qsTr("Unknown Artist");
                card.tertiaryLabelVisible = songInfo.model.album.length !== "";
                card.tertiaryText = qsTr("%1 - track #%2").arg(songInfo.model.album).arg(songInfo.model.albumTrackNo);
            }
        }

        onClosed: {
            timer.stop()
            card.coverSources = [];
            card.primaryText = "";
            card.secondaryText = "";
            card.tertiaryLabelVisible = "";
            card.tertiaryText = "";
        }

        Item {
            id: card
            height: cardColumn.childrenRect.height

            property alias coverSources: coverGrid.covers
            property alias primaryText: primaryLabel.text
            property alias secondaryText: secondaryLabel.text
            property alias tertiaryText: tertiaryLabel.text
            property alias tertiaryLabelVisible: tertiaryLabel.visible

            /* Column containing image and labels */
            Column {
                id: cardColumn
                spacing: units.gu(0.5)
                anchors.top: parent.top
                width: parent.width

                Item {
                    width: parent.width
                    height: coverGrid.height

                    CoverGrid {
                        id: coverGrid
                        size: parent.width
                    }

                    Rectangle {
                        id: iconBg
                        anchors.bottom: coverGrid.bottom
                        anchors.right: coverGrid.right
                        anchors.margins: units.gu(1)
                        height: coverGrid.height * 0.20
                        width: height
                        radius: height / 2
                        color: palette.window
                        opacity: 0.5
                        visible: songInfo.actionsVisible
                    }

                    /* Play button */
                    Icon {
                        id: playerControlsPlayButton
                        anchors.bottom: coverGrid.bottom
                        anchors.right: coverGrid.right
                        anchors.rightMargin: units.gu(1) * 1.6
                        anchors.bottomMargin: units.gu(1) * 1.7
                        visible: songInfo.actionsVisible
                        color: palette.text
                        height: coverGrid.height * 0.15
                        width: height
                        source: "qrc:/images/media-playback-start.svg"
                        onClicked: {
                            trackClicked(songInfo.model); // play track
                            dialog.accept();
                        }
                    }
                }

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: units.gu(1)
                }

                Label {
                    id: primaryLabel
                    anchors {
                        left: parent.left
                        leftMargin: units.gu(1)
                        right: parent.right
                        rightMargin: units.gu(1)
                    }
                    elide: Text.ElideRight
                    font.pointSize: units.fs("large")
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
                    elide: Text.ElideRight
                    font.pointSize: units.fs("medium")
                    opacity: 0.9
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
                    elide: Text.ElideRight
                    font.pointSize: units.fs("medium")
                    opacity: 0.9
                    wrapMode: Text.WordWrap
                }
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 500 }
        }

        Timer {
            id: closingDialogTimer
            interval: 1000
            onTriggered: {
                visible = false
                dialog.close()
            }
        }

        Timer {
            id: timer
            interval: 10000
            onTriggered: {
                opacity = 0
                closingDialogTimer.start()
            }
        }

    }

    function open(model, showActions) {
        songInfo.model = model;
        songInfo.actionsVisible = showActions;
        return dialog.open();
    }
}
