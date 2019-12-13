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

NosonIcon {
    height: units.gu(5)
    width: units.gu(20)
    source: "qrc:/images/media-playback-start.svg"
    label {
        //: this appears in a button with limited space (around 14 characters)
        text: qsTr("Play all")
        font.pointSize: units.fs("medium")
        width: parent.width - units.gu(6)
    }

    property var containerItem

    onClicked: {
        delayPlayAll.start()
    }

    Timer {
        id: delayPlayAll
        interval: 100
        onTriggered: {
            playAll(containerItem)
        }
    }
}
