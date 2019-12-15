/*
 * Copyright (C) 2016-2019
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
import QtQuick.Layouts 1.1
import NosonThumbnailer 1.0
import "../"


DialogBase {
    id: dialog
    title: qsTr("General settings")

    acceptText: qsTr("Ok")
    cancelText: qsTr("Cancel")

    onOpened: {
        apiKey.text = settings.lastfmKey;
    }
    onAccepted: {
        if (settings.lastfmKey !== apiKey.text) {
            settings.lastfmKey = apiKey.text;
            Thumbnailer.reset(); // reset thumbnailer state
            if (apiKey.length > 1) {
                if (Thumbnailer.configure("LASTFM", apiKey.text))
                    thumbValid = true;
            } else {
                if (Thumbnailer.configure("DEEZER", "n/a"))
                    thumbValid = true;
            }
        }
    }
    onRejected: {
        apiKey.text = settings.lastfmKey;
    }

    ColumnLayout {
        id: settingsColumn
        spacing: units.gu(1)

        ColumnLayout {
            visible: true
            spacing: units.gu(0.5)
            Layout.fillWidth: true
            RowLayout {
                spacing: units.gu(1)
                Image {
                    width: units.gu(10)
                    height: units.gu(2.5)
                    fillMode: Image.PreserveAspectCrop
                    source: "qrc:/images/lastfm.png"
                    sourceSize.width: width
                    sourceSize.height: height
                }
                Text {
                    id: link
                    font.pixelSize: units.fx("x-small")
                    text: "<a href='https://www.last.fm/api/account/create'>" + qsTr("Get an API account") + "</a>"
                    onLinkHovered: {
                        if (hoveredLink)
                            font.bold = true;
                        else
                            font.bold = false;
                    }
                    onLinkActivated: Qt.openUrlExternally(link)
                    linkColor: styleMusic.view.linkColor
                }
            }
            TextField {
                id: apiKey
                font.pixelSize: units.fx("medium")
                placeholderText: qsTr("Enter a valid API key");
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
                Layout.fillWidth: true
            }
        }
    }
}
