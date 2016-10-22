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

Item {
    id: refresh
    height: units.gu(5)
    width: parent.width
    visible: false

    anchors {
        horizontalCenter: parent.horizontalCenter
        verticalCenter: parent.verticalCenter
    }

    Rectangle {
        id: loading
        objectName: "LoadingSpinner"
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        height: units.gu(15)
        width: units.gu(15)
        radius: units.gu(1.5)
        color: "transparent"
        visible: refresh.visible

        Rectangle {
            anchors.fill: parent
            border.color: "transparent"
            border.width: parent.border.width
            radius: parent.radius
            color: "white"
            opacity: 0.3
        }
        Image {
            source: "../graphics/working.svg"
            anchors.fill: parent
            anchors.margins: units.gu(2)
        }

        z: 1
    }

    /*
    ActivityIndicator {
        id: loading
        objectName: "LoadingSpinner"
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        running: refresh.visible
        height: units.gu(10)
        width: height
        z: 1
    }
    */
}
