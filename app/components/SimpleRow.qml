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


Row {
    height: units.gu(7)

    property alias column: columnComponent.sourceComponent

    spacing: units.gu(2)

    Loader {
        id: columnComponent
        anchors {
            verticalCenter: parent.verticalCenter
        }
        width: parent.width - parent.spacing

        onSourceComponentChanged: {
            for (var i=0; i < item.children.length; i++) {
                // binds to width so it is updated when screen size changes
                item.children[i].width = Qt.binding(function () { return width; })
            }
        }
    }
}

