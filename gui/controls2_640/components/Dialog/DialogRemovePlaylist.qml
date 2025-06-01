/*
 * Copyright (C) 2016, 2017
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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQml.Models 2.3

DialogBase {
    id: dialog

    footer: Row {
        leftPadding: units.gu(1)
        rightPadding: units.gu(1)
        bottomPadding: units.gu(1)
        spacing: units.gu(1)
        layoutDirection: Qt.RightToLeft

        Button {
            flat: true
            text: qsTr("Cancel")
            onClicked: dialog.reject()
        }
        Button {
            flat: true
            text: qsTr("Ok")
            onClicked: dialog.accept()
        }
    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        //: this is a title of a dialog with a prompt to delete a playlist
        text: qsTr("Permanently delete playlist ?")
        wrapMode: Label.WordWrap
        color: styleMusic.dialog.labelColor
        font.pointSize: units.fs("large")
        font.bold: true
    }

    Text {
        anchors.left: parent.left
        anchors.right: parent.right
        text: qsTr("This cannot be undone.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.foregroundColor
        font.pointSize: units.fs("medium")
        font.weight: Font.Normal
    }
}
