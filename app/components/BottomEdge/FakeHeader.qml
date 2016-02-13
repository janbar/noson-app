/*
 * Copyright (C) 2014-2015 Canonical Ltd
 *
 * This file is part of Ubuntu Clock App
 *
 * Ubuntu Clock App is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Ubuntu Clock App is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import Ubuntu.Components 1.3

Column {
    id: fakeHeader

    height: units.gu(6.125)

    Rectangle {
        height: units.gu(6)
        width: parent.width
        color: theme.palette.normal.background
    }
    
    Rectangle {
        color: "#C9C9C9"
        height: units.gu(0.125)
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
