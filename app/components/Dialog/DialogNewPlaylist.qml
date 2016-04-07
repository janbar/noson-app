/*
 * Copyright (C) 2013, 2014, 2015, 2016
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Andrew Hayzen <ahayzen@gmail.com>
 *      Daniel Holm <d.holmen@gmail.com>
 *      Victor Thompson <victor.thompson@gmail.com>
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
import Ubuntu.Components.Popups 1.3

DialogBase {
    id: dialogNewPlaylist
    objectName: "dialogNewPlaylist"
    title: i18n.tr("New playlist")

    Label {
        id: newplaylistoutput
        color: UbuntuColors.red
        visible: false // should only be visible when an error is made.
        anchors.left: parent.left
        anchors.right: parent.right
        wrapMode: Text.WordWrap
        fontSize: "x-small"
        font.weight: Font.Normal
    }
    TextField {
        id: playlistName
        objectName: "playlistNameTextField"
        placeholderText: i18n.tr("Enter playlist name")
        inputMethodHints: Qt.ImhNoPredictiveText
        color: theme.palette.selected.baseText
    }
    Button {
        text: i18n.tr("Create")
        color: styleMusic.dialog.confirmButtonColor
        objectName: "newPlaylistDialogCreateButton"
        onClicked: {
            newplaylistoutput.visible = false // make sure its hidden now if there was an error last time
            if (playlistName.text.length > 0) { // make sure something is acually inputed
                createPlaylist(playlistName.text)
                PopupUtils.close(dialogNewPlaylist);
            }
            else {
                newplaylistoutput.visible = true
                newplaylistoutput.text = i18n.tr("Please type in a name.")
            }
        }
    }

    Button {
        text: i18n.tr("Cancel")
        color: styleMusic.dialog.cancelButtonColor
        onClicked: PopupUtils.close(dialogNewPlaylist)
    }

    Component.onCompleted: playlistName.forceActiveFocus()
}
