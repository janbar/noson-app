/*
 * Copyright (C) 2015
 *      Andrew Hayzen <ahayzen@gmail.com>
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
import Ubuntu.Components 1.3

Rectangle {
    color: currentColor
    width: height

    Icon {
        anchors {
            centerIn: parent
        }
        objectName: action.objectName
        color: pressed ? UbuntuColors.blue : styleMusic.common.white
        name: action.iconName
        height: units.gu(3)
        width: units.gu(3)
    }

    Rectangle {  // FIXME: pad.lv/1507339 Workaround for gap between end of listitem and first action
        anchors {
            bottom: parent.bottom
            right: parent.left
            top: parent.top
        }
        color: currentColor
        width: units.gu(0.5)
    }
}
