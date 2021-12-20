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


DialogBase {
    id: dialog
    //: this is a title of a dialog with a prompt to add a new playlist
    title: qsTr("New playlist")

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

    Label {
        id: newplaylistoutput
        color: "red"
        visible: false // should only be visible when an error is made.
        anchors.left: parent.left
        anchors.right: parent.right
        wrapMode: Text.WordWrap
        font.pointSize: units.fs("x-small")
        font.weight: Font.Normal
    }

    TextField {
        id: playlistName
        font.pointSize: units.fs("large")
        placeholderText: qsTr("Enter playlist name")
        inputMethodHints: Qt.ImhNone
        EnterKey.type: Qt.EnterKeyDone
    }

    Button {
        height: units.gu(6)
        text: qsTr("Create")
        onClicked: {
            newplaylistoutput.visible = false // make sure its hidden now if there was an error last time
            if (playlistName.text.length > 0) { // make sure something is acually inputed
                createPlaylist(playlistName.text)
                dialog.accept()
            }
            else {
                newplaylistoutput.visible = true
                newplaylistoutput.text = qsTr("Please type in a name.")
            }
        }
    }

    Component.onCompleted: playlistName.forceActiveFocus()
}
