/*
 * Copyright (C) 2019
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


Rectangle {
    id: dataFailureState
    anchors.fill: parent
    color: styleMusic.view.backgroundColor

    signal reloadClicked

    Column {
        anchors.centerIn: parent
        spacing: units.gu(4)
        width: parent.width > units.gu(44) ? parent.width - units.gu(8) : units.gu(40)

        Label {
            color: styleMusic.view.labelColor
            elide: Text.ElideRight
            font.pointSize: units.fs("x-large")
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 2
            text: qsTr("Loading failed")
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Icon {
            id: reload
            anchors.horizontalCenter: parent.horizontalCenter
            visible: true
            source: "qrc:/images/reload.svg"
            height: units.gu(5)
            onClicked: reloadClicked()
            label.text: qsTr("Retry")
        }
    }
}
