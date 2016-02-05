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

Dialog {
    id: dialogSettings
    title: i18n.tr("Settings")

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        text: i18n.tr("Whenever you make changes to your music library, such as adding and removing tracks, or adjusting album artwork, you will need to update the music index on Sonos before these changes will show up in the Sonos controller.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        fontSize: "x-small"
        font.weight: Font.Normal
    }
    Button {
        text: i18n.tr("Update music index now")
        color: UbuntuColors.orange
        onClicked: {
            updateMusicIndex();
            PopupUtils.close(dialogSettings)
        }
    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        text: i18n.tr("Close the settings screen.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        fontSize: "x-small"
        font.weight: Font.Normal
    }
    Button {
        text: i18n.tr("Close")
        color: styleMusic.dialog.cancelButtonColor
        onClicked: PopupUtils.close(dialogSettings)
    }
}
