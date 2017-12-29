/*
 * Copyright (C) 2016, 2017
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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQml.Models 2.2
import "../"

MouseArea {
    id: area

    property color color: "white"
    property color highlightedColor: "lightgray"
    property var currentColor: highlighted ? highlightedColor : color
    property bool highlighted: false

    property alias column: row.column

    anchors { left: parent.left; right: parent.right }
    height: content.height

    Rectangle {
        id: content
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        width: area.width; height: row.implicitHeight + units.dp(4)

        color: area.color
        Behavior on color { ColorAnimation { duration: 100 } }

        // highlight the current position
        Rectangle {
            anchors.fill: row
            visible: area.highlighted
            color: area.highlightedColor
            opacity: 0.2
        }

        SimpleRow {
            id: row
            anchors.fill: parent

            // Animate margin changes so it isn't noticible
            Behavior on anchors.leftMargin {
                NumberAnimation {

                }
            }

            Behavior on anchors.rightMargin {
                NumberAnimation {

                }
            }
        }
    }
}
