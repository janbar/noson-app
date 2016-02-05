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


Rectangle {
    id: playlistsEmptyState
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
            horizontalAlignment: Text.AlignLeft
            maximumLineCount: 2
            text: i18n.tr("No playlists found")
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Label {
            color: styleMusic.libraryEmpty.labelColor
            elide: Text.ElideRight
            fontSize: "large"
            horizontalAlignment: Text.AlignLeft
            maximumLineCount: 4
            text: i18n.tr("Get more out of Sonos by tapping the %1 icon to start making playlists for every mood and occasion.").arg('"+"')
            width: parent.width
            wrapMode: Text.WordWrap
        }
    }
}
