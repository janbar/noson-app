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

Dialog {
    id: dialog

    /*!
      \qmlproperty list<Object> contents
      Content will be put inside a column in the foreround of the Dialog.
    */
    default property alias contents: contentsColumn.data

    /*!
      The property holds the margins from the dialog's dismissArea.
      */
    property real edgeMargins: units.gu(2)

    /*!
      The property controls whether the dialog is modal or not. Modal dialogs block
      event propagation to items under dismissArea, when non-modal ones let these
      events passed to these items. In addition, non-modal dialogs do not dim the
      dismissArea.

      The default value is true.
      */
    modal: true

    /*!
      Grab focus when Dialog is shown
      */
    focus: true


    /*!
      The question to the user.
      \qmlproperty string text
     */
    property string text

    property real minimumWidth: units.gu(mainView.minSizeGU - 2)
    property real minimumHeight: Math.max(256, units.gu(32))
    property real contentSpacing: units.gu(1)

    x: Math.round((mainView.width - width) / 2)
    y: Math.round((mainView.height - (height + header.height + footer.height)) / 2)
    width: Math.max(Math.round(Math.min(mainView.width, mainView.height) / 3 * 2), dialog.minimumWidth)
    readonly property real h: contentsColumn.height + units.gu(6) +
                              2 * dialog.edgeMargins + dialog.header.height + dialog.footer.height
    height: Math.max(Math.min(h, Math.round(mainView.height / 4 * 3)), dialog.minimumHeight)

    Rectangle {
        id: background
        color: "transparent"
        anchors.fill: parent
        anchors.margins: dialog.edgeMargins
    }

    Flickable {
        anchors.fill: parent
        anchors.leftMargin: dialog.edgeMargins
        contentWidth: contentsColumn.width
        contentHeight: contentsColumn.height
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
            visible: (parent.visibleArea.heightRatio < 1.0)
        }

        Column {
            id: contentsColumn
            spacing: dialog.contentSpacing
            width: background.width
            onWidthChanged: updateChildrenWidths();

            Text {
                horizontalAlignment: Text.AlignHCenter
                text: dialog.text
                font.pointSize: units.fs("medium")
                color: styleMusic.dialog.foregroundColor
                wrapMode: Text.Wrap
                visible: (text !== "")
            }

            onChildrenChanged: updateChildrenWidths()

            function updateChildrenWidths() {
                for (var i = 0; i < children.length; i++) {
                    children[i].width = contentsColumn.width;
                }
            }
        }
    }
}
