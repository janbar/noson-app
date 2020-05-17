/*
 * Copyright (C) 2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
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

DialogBase {
    id: dialog
    //: this is a title of a dialog with a prompt to add a new playlist
    title: qsTr("New playlist")

    cancelText: qsTr("Cancel")
    acceptText: qsTr("Create")

    onAccepted: {
        createPlaylist(playlistName.text)
    }

    canAccept: playlistName.text.length > 0

    TextField {
        id: playlistName
        anchors.left: parent.left
        anchors.right: parent.right
        placeholderText: qsTr("Enter playlist name")
        width: dialog.width
    }
}
