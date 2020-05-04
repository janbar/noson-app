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

import QtQuick 2.9
import QtQuick.Controls 2.2
import "../"

Icon {
    height: units.gu(5)
    width: units.gu(20)
    source: "qrc:/images/add.svg"
    label {
        //: this appears in a button with limited space (around 14 characters)
        text: qsTr("Queue all")
        font.pointSize: units.fs("medium")
        width: parent.width - units.gu(6)
        elide: Text.ElideRight
    }

    property var containerItem

    onClicked: {
        delayAddQueue.start()
    }

    Timer {
        id: delayAddQueue
        interval: 100
        onTriggered: {
            addQueue(containerItem)
        }
    }
}
