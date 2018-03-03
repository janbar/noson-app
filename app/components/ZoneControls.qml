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
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/ListItemActions"
import "../components/HeadState"

Item {
    property string controlledZone: ""

    anchors {
        fill: parent
    }

    Rectangle {
        id: notImplemented
        anchors {
            fill: parent
        }
        color: mainView.backgroundColor

        Column {
            anchors.centerIn: parent
            spacing: units.gu(4)
            width: units.gu(36)

            Label {
                color: styleMusic.libraryEmpty.labelColor
                elide: Text.ElideRight
                fontSize: "x-large"
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 1
                text: i18n.tr("About")
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                color: styleMusic.libraryEmpty.labelColor
                elide: Text.ElideRight
                fontSize: "normal"
                horizontalAlignment: Text.AlignLeft
                maximumLineCount: 10
                text: i18n.tr("The project has started in 2015 and is intented to make a fast and smart controller for your SONOS devices. You can browse your music library and play track or radio on any zones. You can manage grouping zones, queue, and playlists, and fully control the playback.")
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                color: styleMusic.libraryEmpty.labelColor
                elide: Text.ElideRight
                fontSize: "normal"
                horizontalAlignment: Text.AlignLeft
                maximumLineCount: 1
                text: i18n.tr("Version: %1").arg(mainView.versionString)
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                color: styleMusic.libraryEmpty.labelColor
                elide: Text.ElideRight
                fontSize: "normal"
                horizontalAlignment: Text.AlignLeft
                maximumLineCount: 1
                text: i18n.tr("Author: %1").arg("Jean-Luc Barriere")
                width: parent.width
                wrapMode: Text.WordWrap
            }

        }
    }
}
