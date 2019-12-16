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
import QtGraphicalEffects 1.0

MouseArea {
    property var model

    height: units.gu(4)

    onClicked: delayShuffleModel.start()

    Timer {
        id: delayShuffleModel
        interval: 100
        onTriggered: {
            shuffleModel(model)
        }
    }

    Image {
        id: icon
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height
        width: height
        sourceSize.height: height
        sourceSize.width: width
        source: "qrc:/images/media-playlist-shuffle.svg"
    }
    BrightnessContrast {
        anchors.fill: icon
        source: icon
        /*!FIXME: brightness: 0.5+styleMusic.view.primaryColor.hslLightness*/
        brightness: Qt.colorEqual(styleMusic.view.primaryColor, "black") ? -0.5 : +0.5
    }

    Text {
        color: styleMusic.view.primaryColor
        anchors.left: icon.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: units.gu(2)
        anchors.topMargin: units.gu(1)
        //: this appears in a button with limited space (around 14 characters)
        text: qsTr("Shuffle")
        font.pixelSize: units.fx("medium")
        elide: Text.ElideRight
        width: parent.width - icon.width - anchors.leftMargin
    }
}
