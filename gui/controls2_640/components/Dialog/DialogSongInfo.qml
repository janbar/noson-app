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
import QtQuick.Controls 2.2
import Qt5Compat.GraphicalEffects 6.0
import NosonApp 1.0
import "../Delegates"
import "../"

Item {
    id: songInfo
    property var model: null
    property string moreSource: ""
    property var moreArgs: ({})
    property bool forceActionMore: false
    property bool canPlay: false
    property bool canQueue: false
    property bool isContainer: false

    DialogBase {
        id: dialog

        footer: Row {
            leftPadding: units.gu(1)
            rightPadding: units.gu(1)
            bottomPadding: units.gu(1)
            spacing: units.gu(1)
            layoutDirection: Qt.RightToLeft

            Button {
                flat: true
                text: qsTr("Close")
                onClicked: dialog.close()
            }
            Button {
                id: buttonPlay
                flat: true
                text: qsTr("Play")
                visible: false
                onClicked: dialog.accept()
            }
            Button {
                id: buttonMore
                flat: true
                text: qsTr("More")
                visible: false
                onClicked: {
                    dialog.close();
                    stackView.push(dialogSongInfo.moreSource, dialogSongInfo.moreArgs);
                }
            }
        }

        width: mainView.minimumWidth - units.gu(2)

        onOpened: {
            timer.restart();
            if (songInfo.model) {
                primaryText = songInfo.model.title !== "" ? songInfo.model.title : qsTr("Unknown Album");
                secondaryText = songInfo.model.author !== "" ? songInfo.model.author : qsTr("Unknown Artist");
                tertiaryLabelVisible = (songInfo.model.album.length > 0);
                if (songInfo.model.albumTrackNo && songInfo.model.albumTrackNo > 0) {
                    if (songInfo.model.albumDiscNo && songInfo.model.albumDiscNo > 0)
                        tertiaryText = qsTr("%1 - track #%2.%3").arg(songInfo.model.album).arg(songInfo.model.albumDiscNo).arg(songInfo.model.albumTrackNo);
                    else
                        tertiaryText = qsTr("%1 - track #%2").arg(songInfo.model.album).arg(songInfo.model.albumTrackNo);
                } else if (songInfo.model.description) {
                    tertiaryText = songInfo.model.description;
                } else {
                    tertiaryText = songInfo.model.album;
                }
            }

            if (canPlay) {
                buttonPlay.visible = true;
            } else {
                buttonPlay.visible = false;
            }
            // do not stack more than one page for library view
            // do not show the artist view for an item of service
            if (moreSource.length > 0 && (forceActionMore || !Sonos.isItemFromService(songInfo.model.payload))) {
                buttonMore.visible = true;
            } else {
                buttonMore.visible = false;
            }
        }

        onAccepted: {
            if (canPlay) {
                if (canQueue) {
                    if (isContainer)
                        playAll(songInfo.model);
                    else
                        trackClicked(songInfo.model);
                } else {
                    radioClicked(songInfo.model);
                }
            }
        }

        onClosed: {
            timer.stop()
            primaryText = "";
            secondaryText = "";
            tertiaryLabelVisible = "";
            tertiaryText = "";
            buttonMore.visible = false;
            buttonPlay.visible = false;
        }

        property alias primaryText: primaryLabel.text
        property alias secondaryText: secondaryLabel.text
        property alias tertiaryText: tertiaryLabel.text
        property alias tertiaryLabelVisible: tertiaryLabel.visible

        /* Column for labels */
        Column {
            spacing: units.gu(0.5)

            Label {
                id: primaryLabel
                anchors {
                    left: parent.left
                    right: parent.right
                }
                color: styleMusic.dialog.labelColor
                font.pointSize: units.fs("large")
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
                font.pointSize: units.fs("medium")
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
                font.pointSize: units.fs("medium")
                opacity: 0.9
                wrapMode: Text.WordWrap
                font.italic: true
            }
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
                closingDialogTimer.start()
            }
        }

    }

    function open(model, art, moreSource, moreArgs, forceActionMore, canPlay, canQueue, isContainer) {
        songInfo.model = model;
        songInfo.moreSource = moreSource;
        songInfo.moreArgs = moreArgs;
        songInfo.forceActionMore = (forceActionMore ? true : false);
        songInfo.canPlay = (canPlay ? true : false);
        songInfo.canQueue = (canQueue ? true : false);
        songInfo.isContainer = (isContainer ? true : false);
        return dialog.open();
    }
}
