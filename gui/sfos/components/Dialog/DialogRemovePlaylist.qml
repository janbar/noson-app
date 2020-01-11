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

Item {
    id: removingPlaylist
    property var containerItem: null
    property alias result: dialog.result
    property alias status: dialog.status
    signal done

    DialogBase {
        id: dialog
        title: qsTr("Permanently delete playlist ?")

        acceptText: qsTr("Ok")
        cancelText: qsTr("Cancel")

        onDone: removingPlaylist.done()

        onAccepted: {
            // removing playlist
            removeFromFavorites(removingPlaylist.containerItem.payload)
            removePlaylist(removingPlaylist.containerItem.id)
        }

        Label {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            //: this is a title of a dialog with a prompt to delete a playlist
            text: qsTr("This cannot be undone.")
            wrapMode: Label.WordWrap
            color: styleMusic.dialog.labelColor
            font.pixelSize: units.fx("large")
        }
   }

    function open(containerItem) {
        removingPlaylist.containerItem = containerItem;
        return dialog.open();
    }
}
