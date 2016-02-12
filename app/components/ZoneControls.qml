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
                maximumLineCount: 2
                text: i18n.tr("Under construction")
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                color: styleMusic.libraryEmpty.labelColor
                elide: Text.ElideRight
                fontSize: "large"
                horizontalAlignment: Text.AlignLeft
                maximumLineCount: 4
                text: i18n.tr("This feature will be added in a next version. Please rate the App.")
                width: parent.width
                wrapMode: Text.WordWrap
            }
        }
    }
}
