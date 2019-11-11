/*
 * Copyright (C) 2014, 2015, 2016
 *      Andrew Hayzen <ahayzen@gmail.com>
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
import "../"


Item {
    id: card
    height: parent.parent.cellHeight
    width: parent.parent.cellWidth

    property alias color: bg.color
    property alias coversGridVisible: coverGrid.visible
    property alias coverSources: coverGrid.covers
    property alias coverFlow: coverGrid.flowModel
    property alias imageSource: coverGrid.firstSource
    property alias noCover: coverGrid.noCover
    property alias overlay: coverGrid.overlay
    property alias primaryText: primaryLabel.text
    property alias secondaryText: secondaryLabel.text
    property alias secondaryTextVisible: secondaryLabel.visible
    property bool isFavorite: false
    property bool canPlay: false

    signal clicked(var mouse)
    signal pressAndHold(var mouse)
    signal playClicked(var mouse)

    signal imageError(var index)

    /* Animations */
    Behavior on height {
        NumberAnimation {
            duration: 250
        }
    }

    Behavior on width {
        NumberAnimation {
            duration: 250
        }
    }

    /* Background for card */
    Rectangle {
        id: bg
        anchors {
            fill: parent
            margins: units.gu(1)
        }
        border.width: units.dp(1)
        border.color: styleMusic.card.borderColor
        color: styleMusic.card.backgroundColor
        opacity: 0.1
    }

    /* Column containing image and labels */
    Column {
        id: cardColumn
        anchors {
            fill: bg
        }
        spacing: units.gu(0.5)

        CoverGrid {
            id: coverGrid
            size: parent.width
            onImageError: card.imageError(index)
        }

        // Adjust spacing
        Item {
            height: units.gu(0.5)
            width: parent.width
        }

        // Labels are ~1.5GU per line
        // We are limiting to 3 lines long
        // with the preference on the first label
        Label {
            id: primaryLabel
            anchors {
                left: parent.left
                leftMargin: units.gu(1)
                right: parent.right
                rightMargin: units.gu(1)
            }
            color: styleMusic.view.primaryColor
            elide: Text.ElideRight
            font.pointSize: units.fs("small")
            maximumLineCount: 2
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }

        Label {
            id: secondaryLabel
            anchors {
                left: parent.left
                leftMargin: units.gu(1)
                right: parent.right
                rightMargin: units.gu(1)
            }
            color: styleMusic.view.secondaryColor
            elide: Text.ElideRight
            font.pointSize: units.fs("small")
            // Allow wrapping of 2 lines unless primary has been wrapped
            maximumLineCount: primaryLabel.lineCount > 1 ? 1 : 2
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }
    }

    /* Show play */
    NosonIcon {
        id: playMouseArea
        anchors {
            bottom: bg.bottom
            right: bg.right
        }
        height: canPlay ? (card.width * 0.40) : 0
        width: height
        enabled: canPlay
        opacity: 0.5
        onClicked: card.playClicked(mouse)
        //onPressAndHold: card.pressAndHold(mouse)
        source: "qrc:/images/media-preview-start.svg"
        //color: styleMusic.card.foregroundColor
    }

    /* Show starred */
    NosonIcon {
        id: starMouseArea
        anchors {
            bottom: playMouseArea.top
            right: bg.right
        }
        height: isFavorite ? (card.width * 0.40) : 0
        width: height
        enabled: isFavorite
        source: "qrc:/images/starred.svg"
        //color: styleMusic.card.foregroundColor
    }

    /* Overlay for when card is pressed */
    Rectangle {
        id: overlay
        anchors {
            fill: bg
        }
        color: styleMusic.card.backgroundColor
        opacity: cardMouseArea.pressed ? 0.3 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: 250
            }
        }
    }

    /* Capture mouse events */
    MouseArea {
        id: cardMouseArea
        anchors {
            fill: parent
            bottomMargin: parent.height * 0.25 // do not override action icons
        }
        onClicked: card.clicked(mouse)
        //onPressAndHold: card.pressAndHold(mouse)
    }
}

