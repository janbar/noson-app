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
import Ubuntu.Components 1.2
import Ubuntu.Components.Popups 1.0

Dialog {
    id: dialogRemovePlaylist
    objectName: "dialogRemovePlaylist"
    // TRANSLATORS: this is a title of a dialog with a prompt to delete a playlist
    title: i18n.tr("Permanently delete playlist ?")

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        text: i18n.tr("This cannot be undone")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        fontSize: "x-small"
        font.weight: Font.Normal
    }

    property string oldPlaylistId

    Button {
        text: i18n.tr("Remove")
        color: styleMusic.dialog.confirmRemoveButtonColor
        objectName: "removePlaylistDialogRemoveButton"
        onClicked: {
            // removing playlist
            removePlaylist(dialogRemovePlaylist.oldPlaylistId)
            PopupUtils.close(dialogRemovePlaylist)

            // need to destroy the dialog before popping fixes pad.lv/1428450
            dialogRemovePlaylist.destroy()

            mainPageStack.goBack()
        }
    }
    Button {
        text: i18n.tr("Cancel")
        color: styleMusic.dialog.cancelButtonColor
        onClicked: PopupUtils.close(dialogRemovePlaylist)
    }
}
