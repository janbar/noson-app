/*
 * Copyright 2015 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3

PopupBase {
    id: dialog

    /*!
      \qmlproperty list<Object> contents
      Content will be put inside a column in the foreround of the Dialog.
    */
    default property alias contents: contentsColumn.data

    /*!
      The title of the question to ask the user.
      \qmlproperty string title
     */
    property alias title: foreground.title

    /*!
      The question to the user.
      \qmlproperty string text
     */
    property alias text: foreground.text

    /*!
      The Item such as a \l Button that the user interacted with to open the Dialog.
      This property will be used for the automatic positioning of the Dialog next to
      the caller, if possible.
     */
    property Item caller

    /*!
      The property holds the margins from the dialog's dismissArea. The property
      is themed.
      */
    property real edgeMargins: units.gu(2)

    /*!
      The property holds the margin from the dialog's caller. The property
      is themed.
      */
    property real callerMargin: units.gu(1)

    /*!
      The property controls whether the dialog is modal or not. Modal dialogs block
      event propagation to items under dismissArea, when non-modal ones let these
      events passed to these items. In addition, non-modal dialogs do not dim the
      dismissArea.

      The default value is true.
      */
    property bool modal: true

    __foreground: foreground
    __eventGrabber.enabled: modal
    __dimBackground: modal
    fadingAnimation: UbuntuNumberAnimation { duration: UbuntuAnimation.SnapDuration }

    StyledItem {
        id: foreground
        // Grab focus when Dialog is shown
        focus: visible
        width: Math.min(minimumWidth, dialog.width)
        anchors.centerIn: parent

        // used in the style
        property string title
        property string text
        property real minimumWidth: units.gu(38)
        property real minimumHeight: units.gu(32)
        property real maxHeight: 3*dialog.height/4
        property real margins: units.gu(4)
        property real itemSpacing: units.gu(2)
        property Item dismissArea: dialog.dismissArea

        height: Math.min(contentsColumn.height + foreground.margins, dialog.height)

        Rectangle {
            id: background
            color: styleMusic.dialog.backgroungColor
            anchors.fill: parent
            radius: 10
        }

        Flickable {
            anchors.fill: parent
            anchors.margins: foreground.margins
            contentWidth: contentsColumn.width
            contentHeight: contentsColumn.height - foreground.margins
            boundsBehavior: Flickable.StopAtBounds

            Column {
                id: contentsColumn
                spacing: foreground.itemSpacing
                width: foreground.width - foreground.margins * 2
                height: childrenRect.height + foreground.margins
                onWidthChanged: updateChildrenWidths();

                Label {
                    horizontalAlignment: Text.AlignHCenter
                    text: dialog.title
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    fontSize: "large"
                    color: styleMusic.dialog.labelColor
                    visible: (text !== "")
                }

                Label {
                    horizontalAlignment: Text.AlignHCenter
                    text: dialog.text
                    fontSize: "medium"
                    color: styleMusic.dialog.labelColor
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

        styleName: "DialogForegroundStyle"
    }
}
