/*
 * Copyright (C) 2020
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

import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

DialogBase {
    id: dialog
    //: this is a title of a dialog to add a new service
    title: qsTr("Add Service")

    property alias label: inputLabel.text
    property string serviceType: ""
    property string serviceIcon: ""

    footer: Row {
        leftPadding: units.gu(1)
        rightPadding: units.gu(1)
        bottomPadding: units.gu(1)
        spacing: units.gu(1)
        layoutDirection: Qt.RightToLeft

        Button {
            flat: true
            text: qsTr("Ok")
            onClicked: dialog.accept()
        }
    }

    Image {
        source: serviceIcon
        visible: source !== ""
        height: units.gu(15)
        fillMode: Qt.KeepAspectRatio
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        font.pointSize: units.fs("small")
        font.weight: Font.Normal
        text: qsTr("You can specify a label for the new service to differentiate it from an existing one for the same provider.")
    }

    TextField {
        id: inputLabel
        font.pointSize: units.fs("medium")
        placeholderText: qsTr("Enter the label")
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
        EnterKey.type: Qt.EnterKeyDone
    }

    onOpened: {
        label = "";
    }

}
