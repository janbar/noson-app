/*
 * Copyright (C) 2015-2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
 *      Andrew Hayzen <ahayzen@gmail.com>
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

SilicaGridView {
    id: gridView
    anchors.fill: parent
    cellHeight: cellSize + heightOffset
    cellWidth: cellSize + widthOffset

    VerticalScrollDecorator { flickable: listView }

    readonly property int columns: parseInt(width / itemWidth) || 1  // never drop to 0
    readonly property int cellSize: width / columns
    property int itemWidth: units.gu(15)
    property int heightOffset: 0
    property int widthOffset: 0

    add: Transition {
        NumberAnimation { properties: "x,y"; duration: 200 }
    }
    removeDisplaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 200 }
    }
    addDisplaced: Transition {
        NumberAnimation { properties: "x,y"; duration: 200 }
    }
}
