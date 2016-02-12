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
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3

Dialog {
    id: dialogManageQueue
    objectName: "dialogManageQueue"
    // TRANSLATORS: this is a title of a dialog to manage queue
    title: i18n.tr("Manage queue")

    Label {
        id: newplaylistoutput
        anchors.left: parent.left
        anchors.right: parent.right
        wrapMode: Text.WordWrap
        color: UbuntuColors.red
        fontSize: "x-small"
        font.weight: Font.Normal
        visible: false // should only be visible when an error is made.
    }
    TextField {
        id: playlistName
        placeholderText: i18n.tr("Enter playlist name")
        inputMethodHints: Qt.ImhNoPredictiveText
    }
    Button {
        text: i18n.tr("Save queue")
        color: UbuntuColors.green
        onClicked: {
            newplaylistoutput.visible = false // make sure its hidden now if there was an error last time
            if (playlistName.text.length > 0) { // make sure something is acually inputed
                if (saveQueue(playlistName.text))
                    PopupUtils.close(dialogManageQueue);
                else {
                    newplaylistoutput.color = UbuntuColors.red
                    newplaylistoutput.text = i18n.tr("Saving failed.")
                }
            }
            else {
                newplaylistoutput.visible = true
                newplaylistoutput.text = i18n.tr("Please type in a name.")
            }
        }
    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        text: i18n.tr("Clearing the queue cannot be undone.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        fontSize: "x-small"
        font.weight: Font.Normal
    }
    Button {
        text: i18n.tr("Clear queue")
        color: styleMusic.dialog.confirmRemoveButtonColor
        onClicked: {
            // clearing queue
            removeAllTracksFromQueue()
            PopupUtils.close(dialogManageQueue)
        }
    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        text: i18n.tr("Close the queue management screen.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        fontSize: "x-small"
        font.weight: Font.Normal
    }
    Button {
        text: i18n.tr("Close")
        color: styleMusic.dialog.cancelButtonColor
        onClicked: PopupUtils.close(dialogManageQueue)
    }
}
