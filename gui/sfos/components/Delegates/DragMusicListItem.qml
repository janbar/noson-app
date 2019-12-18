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

MouseArea {
    id: dragArea
    property ListView listview: null
    property color color: styleMusic.view.backgroundColor
    property color highlightedColor: styleMusic.view.highlightedColor
    property var currentColor: highlighted ? highlightedColor : color
    property bool highlighted: false
    property bool reorderable: true

    signal swipe
    signal click
    signal actionPressed
    signal action2Pressed
    signal action3Pressed
    signal reorder(int from, int to)

    signal imageError

    Connections {
        target: row
        onActionPressed: actionPressed()
        onAction2Pressed: action2Pressed()
        onAction3Pressed: action3Pressed()
    }

    property alias coverSize: row.coverSize
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
    property alias action3Visible: row.action3Visible
    property alias action3IconSource: row.action3IconSource
    property alias menuVisible: row.menuVisible
    ///property alias menu: row.menu

    property bool held: false

    anchors { left: parent.left; right: parent.right }
    //contentHeight: content.height
    height: content.height

    drag.target: held ? content : undefined
    drag.axis: Drag.YAxis

    /* Detect row swipe */
    property string direction: "None"
    property real lastX: -1
    property real lastY: -1

    onPressed: {
        lastX = mouse.x
        lastY = mouse.y
    }

    onPressAndHold: {
        if (reorderable)
            held = true
    }
    onReleased: {
        if (held) {
            held = false;
        } else {
            var diffX = mouse.x - lastX;
            if (Math.abs(diffX) > units.gu(10)) {
                swipe();
            } else {
                click();
            }
        }
    }

    Rectangle {
        id: content
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        width: dragArea.width; height: row.contentHeight + units.dp(4)

        color: held ? dragArea.highlightedColor : dragArea.color
        Behavior on color { ColorAnimation { duration: 100 } }

        radius: 0
        Drag.active: held
        Drag.source: dragArea
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        states: State {
            when: held

            ParentChange { target: content; parent: listview.parent }
            AnchorChanges {
                target: content
                anchors { horizontalCenter: undefined; verticalCenter: undefined }
            }
        }

        // highlight the current position
        Rectangle {
            anchors.centerIn: parent
            width: dragArea.width
            height: dragArea.height
            visible: dragArea.highlighted
            color: dragArea.highlightedColor
            opacity: 0.4
        }

        MusicRow {
            id: row
            anchors.fill: parent
            onImageError: dragArea.imageError()
        }
    }

    DropArea {
        anchors { fill: parent; margins: 10 }

        onEntered: {
            // save positions
            if (dragArea.sourceIndex < 0)
                dragArea.sourceIndex = drag.source.DelegateModel.itemsIndex;
            dragArea.targetIndex = dragArea.DelegateModel.itemsIndex;

            listview.model.items.move(
                    drag.source.DelegateModel.itemsIndex,
                    dragArea.DelegateModel.itemsIndex);
        }
    }
    property int sourceIndex: -1
    property int targetIndex: -1

    onHeldChanged: {
        if (!held && sourceIndex >= 0) {
            reorder(sourceIndex, targetIndex);
            sourceIndex = targetIndex = -1;
        }
    }
}
