/*
 * Copyright (C) 2018
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
import QtQuick.Layouts 1.3

Rectangle {
    id: filter
    visible: true
    width: parent.width
    height: visible ? units.gu(6) : 0
    color: styleMusic.playerControls.backgroundColor

    property alias text: field.text

    RowLayout {
        spacing: 0
        anchors.fill: parent
        anchors.rightMargin: units.gu(1.5)

        Item {
            width: units.gu(6)
            height: width

            Icon {
                width: units.gu(5)
                height: width
                anchors.centerIn: parent
                source: "qrc:/images/edit-clear.svg"
                onClicked: {
                    filter.text = "";
                    filter.visible = false;
                }
            }
        }

        TextField {
            id: field
            Layout.fillWidth: true
            font.pointSize: units.fs("large")
            inputMethodHints: Qt.ImhNoPredictiveText
            placeholderText: qsTr("Search music")
            EnterKey.type: Qt.EnterKeySearch
        }
    }

    onVisibleChanged: {
        if (visible) {
            field.enabled = true;
            field.forceActiveFocus();
        } else {
            field.enabled = false;
        }
    }
}
