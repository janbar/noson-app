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
import QtQuick.Layouts 1.1
import Sailfish.Silica 1.0
import QtQml.Models 2.3

DialogBase {
    id: dialog
    title: qsTr("Sonos settings")

    cancelText: qsTr("Close")
    acceptText: ""
    canAccept: false

    contentSpacing: units.gu(3)

    Text {
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("Whenever you make changes to your music library, such as adding and removing tracks, or adjusting album artwork, you will need to update the music index on Sonos before these changes will show up in the Sonos controller.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.foregroundColor
        font.pixelSize: units.fx("medium")
        font.weight: Font.Normal
    }

    Button {
        anchors.horizontalCenter: parent.horizontalCenter
        height: units.gu(6)
        text: qsTr("Update music index now")
        onClicked: {
            updateMusicIndex();
            dialog.accept()
        }
        enabled: !shareIndexInProgress
    }

}
