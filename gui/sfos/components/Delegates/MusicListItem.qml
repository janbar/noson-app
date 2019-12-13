/*
 * Copyright (C) 2017
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

import QtQuick 2.2
import Sailfish.Silica 1.0
import "../"

ListItem {
    id: area

    property color color: styleMusic.view.backgroundColor
    property color highlightedColor: styleMusic.view.highlightedColor
    property var currentColor: highlighted ? highlightedColor : color
    property bool highlighted: false

    signal swipe
    signal click
    signal actionPressed

    signal imageError

    Connections {
        target: row
        onActionPressed: actionPressed()
    }

    property alias column: row.column
    property alias description: row.description
    property alias isFavorite: row.isFavorite
    property alias imageSources: row.imageSources
    property alias imageSource: row.imageSource
    property alias noCover: row.noCover
    property alias actionVisible: row.actionVisible
    property alias actionIconSource: row.actionIconSource
    property alias action2Visible: row.action2Visible
    property alias action2IconSource: row.action2IconSource
    property alias menuVisible: row.menuVisible

    anchors { left: parent.left; right: parent.right }
    contentHeight: content.height
    width: ListView.view.width

    /* Detect row swipe */
    property string direction: "None"
    property real lastX: -1    
    property real lastY: -1

    onPressed: {
        lastX = mouse.x
        lastY = mouse.y
    }

    onReleased: {
        var diffX = mouse.x - lastX;
        if (Math.abs(diffX) > units.gu(15)) {
            swipe();
        } else {
            //click();
        }
    }

    Rectangle {
        id: content
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        width: area.width; height: row.contentHeight + units.dp(4)

        color: area.color
        Behavior on color { ColorAnimation { duration: 100 } }

        // highlight the current position
        Rectangle {
            anchors.centerIn: parent
            width: area.width
            height: area.height
            visible: area.highlighted
            color: area.highlightedColor
            opacity: 0.4
        }

        MusicRow {
            id: row
            anchors.fill: parent
            onImageError: area.imageError()
        }
    }
}
