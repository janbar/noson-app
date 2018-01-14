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
import Ubuntu.Components.Popups 1.3

DialogBase {
    id: dialogSelectSource
    objectName: "dialogSelectSource"
    // TRANSLATORS: this is a title of a dialog to select source
    title: i18n.tr("Select source")

    Label {
        id: sourceOutput
        anchors.left: parent.left
        anchors.right: parent.right
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        fontSize: "x-small"
        font.weight: Font.Normal
        visible: false // should only be visible when an error is made.
    }

    TextField {
        id: url
        text: inputStreamUrl
        placeholderText: i18n.tr("Enter stream URL")
        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
        color: theme.palette.selected.baseText
    }

    Connections {
        target: player.zonePlayer
        onJobFailed: {
            sourceOutput.color = UbuntuColors.red
            sourceOutput.text = i18n.tr("Playing failed.")
            sourceOutput.visible = true
        }
    }

    Button {
        id: buttonPlayStream
        text: i18n.tr("Play stream")
        color: UbuntuColors.green
        onClicked: {
            sourceOutput.visible = false // make sure its hidden now if there was an error last time
            if (url.text.length > 0) { // make sure something is acually inputed
                color = UbuntuColors.lightGrey
                if (player.playStream(url.text, "")) {
                    inputStreamUrl = url.text
                }
                else {
                    sourceOutput.color = UbuntuColors.red
                    sourceOutput.text = i18n.tr("Playing failed.")
                    sourceOutput.visible = true
                }
                color = UbuntuColors.green
            }
            else {
                sourceOutput.visible = true
                sourceOutput.text = i18n.tr("Please type in an URL.")
            }
        }
    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        text: i18n.tr("Select the audio input.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        fontSize: "x-small"
        font.weight: Font.Normal
    }

    Button {
        text: i18n.tr("Play line IN")
        color: UbuntuColors.orange
        onClicked: {
            if (!player.playLineIN())
                popInfo.open(i18n.tr("Action can't be performed"))
            else
                PopupUtils.close(dialogSelectSource)
        }
    }

    Button {
        text: i18n.tr("Play TV")
        color: UbuntuColors.orange
        onClicked: {
            if (!player.playDigitalIN())
                popInfo.open(i18n.tr("Action can't be performed"))
            else
                PopupUtils.close(dialogSelectSource)
        }
    }

    Button {
        text: i18n.tr("Queue")
        color: UbuntuColors.orange
        onClicked: {
            if (!player.playQueue(false))
                popInfo.open(i18n.tr("Action can't be performed"))
            else
                PopupUtils.close(dialogSelectSource)
        }
    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        text: i18n.tr("Close this screen.")
        wrapMode: Text.WordWrap
        color: styleMusic.dialog.labelColor
        fontSize: "x-small"
        font.weight: Font.Normal
    }
    Button {
        text: i18n.tr("Close")
        color: styleMusic.dialog.cancelButtonColor
        onClicked: PopupUtils.close(dialogSelectSource)
    }
}
