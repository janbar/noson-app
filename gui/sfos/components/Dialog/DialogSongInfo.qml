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

import QtQuick 2.0
import Sailfish.Silica 1.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import NosonApp 1.0
import "../Delegates"
import "../"

Item {
    id: songInfo
    property var model: null
    property var covers: []
    property string moreSource: ""
    property var moreArgs: ({})
    property bool forceActionMore: false
    property alias status: dialog.status
    signal showMore

    DialogBase {
        id: dialog
        acceptText: qsTr("Play")
        cancelText: qsTr("Close")
        canAccept: true

        contentSpacing: units.gu(3)

        onDone: {
            if (result === DialogResult.None)
                showMore();
        }

        onOpened: {
            if (songInfo.model) {
                card.coverSources = covers
                card.primaryText = songInfo.model.title !== "" ? songInfo.model.title : qsTr("Unknown Album");
                card.secondaryText = songInfo.model.author !== "" ? songInfo.model.author : qsTr("Unknown Artist");
                card.tertiaryLabelVisible = (songInfo.model.album.length > 0);
                if (songInfo.model.albumTrackNo) {
                    card.tertiaryText = qsTr("%1 - track #%2").arg(songInfo.model.album).arg(songInfo.model.albumTrackNo);
                } else if (songInfo.model.description) {
                    card.tertiaryText = songInfo.model.description;
                } else {
                    card.tertiaryText = songInfo.model.album;
                }
            }

            // do not stack more than one page for artist view
            // do not show the artist view for an item of service
            if (forceActionMore || (
                    !pageStack.find(function(page) { return page.objectName === "artistViewPage"; }) &&
                    !Sonos.isItemFromService(songInfo.model.payload))) {
                buttonMore.visible = true;
            } else {
                buttonMore.visible = false;
            }
        }

        onAccepted: {
            trackClicked(songInfo.model); // play track
        }

        onRejected: {
            card.coverSources = [];
            card.primaryText = "";
            card.secondaryText = "";
            card.tertiaryLabelVisible = "";
            card.tertiaryText = "";
            buttonMore.visible = false;
        }

        Item {
            id: card
            height: coverGrid.height
            width: parent.width

            property alias coverSources: coverGrid.covers
            property alias primaryText: primaryLabel.text
            property alias secondaryText: secondaryLabel.text
            property alias tertiaryText: tertiaryLabel.text
            property alias tertiaryLabelVisible: tertiaryLabel.visible


            CoverGrid {
                id: coverGrid
                size: parent.width
                noCover: ""

                Rectangle {
                    id: labelsBackground
                    anchors.bottom: parent.bottom
                    height: labels.height < units.gu(6) ? units.gu(9)
                                                         : labels.height + units.gu(3)
                    width: parent.width
                    color: styleMusic.dialog.backgroundColor
                    opacity: 0.60
                    clip: true
                }

                /* Column for labels */
                Column {
                    id: labels
                    anchors {
                        left: parent.left
                        leftMargin: units.gu(1.5)
                        right: parent.right
                        rightMargin: units.gu(1.5)
                        bottom: parent.bottom
                        bottomMargin: units.gu(1.5)
                    }

                    spacing: units.gu(0.5)

                    Label {
                        id: primaryLabel
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        color: styleMusic.dialog.labelColor
                        elide: Text.ElideRight
                        font.pixelSize: units.fx("large")
                        font.bold: true
                        opacity: 1.0
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: secondaryLabel
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        color: styleMusic.dialog.labelColor
                        elide: Text.ElideRight
                        font.pixelSize: units.fx("medium")
                        opacity: 0.9
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        id: tertiaryLabel
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        color: styleMusic.dialog.labelColor
                        elide: Text.ElideRight
                        font.pixelSize: units.fx("medium")
                        opacity: 0.9
                        wrapMode: Text.WordWrap
                        font.italic: true
                    }
                }
            }
        }

        Behavior on opacity {
            NumberAnimation { duration: 500 }
        }

        Button {
            id: buttonMore
            visible: false
            anchors.horizontalCenter: parent.horizontalCenter
            //: this appears in a button with limited space (around 30 characters)
            text: qsTr("More")
            onClicked: {
                // returning result DialogResult.None
                dialog.close();
            }
        }
    }

    function open(model, covers, moreSource, moreArgs, forceActionMore) {
        songInfo.model = model;
        songInfo.covers = covers;
        songInfo.moreSource = moreSource;
        songInfo.moreArgs = moreArgs;
        songInfo.forceActionMore = (forceActionMore ? true : false);
        return dialog.open();
    }
}
