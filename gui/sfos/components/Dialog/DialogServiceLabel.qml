/*
 * Copyright (C) 2019
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Adam Pigg <adam@piggz.co.uk>
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

Item {
    id: serviceLabel
    property alias label: inputLabel.text
    property string serviceType: ""
    property string serviceIcon: ""

    property alias result: dialog.result
    property alias status: dialog.status
    signal done

    DialogBase {
        id: dialog

        acceptText: qsTr("Ok")
        cancelText: qsTr("Cancel")

        onDone: serviceLabel.done()

        Image {
            source: serviceIcon
            visible: source !== ""
            height: units.gu(15)
            width: height
            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
            sourceSize.height: 512
            sourceSize.width: 512
        }

        Label {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            wrapMode: Text.WordWrap
            color: styleMusic.dialog.labelColor
            font.pixelSize: units.fx("small")
            font.weight: Font.Normal
            text: qsTr("You can specify a label for the new service to differentiate it from an existing one for the same provider.")
        }

        TextField {
            id: inputLabel
            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: units.fx("medium")
            placeholderText: qsTr("Enter the label")
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
        }
   }

    function open(serviceType, serviceIcon) {
        serviceLabel.label = "";
        serviceLabel.serviceType = serviceType;
        serviceLabel.serviceIcon = serviceIcon;
        return dialog.open();
    }
}
