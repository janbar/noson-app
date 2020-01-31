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
import QtQuick.Layouts 1.1
import NosonApp 1.0
import NosonThumbnailer 1.0
import "../"


DialogBase {
    id: dialog
    title: qsTr("General settings")

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

    SectionHeader { text: qsTr("Thumbnails") }

    Text {
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("By default thumbnails are downloaded from %1. You can configure another provider by specifying the API key.")
                .arg("<b>deezer</b>")
        font.pixelSize: units.fx("medium")
        wrapMode: Text.WordWrap
        color: styleMusic.view.foregroundColor
    }

    ColumnLayout {
        visible: true
        spacing: units.gu(1)
        RowLayout {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
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
                Layout.fillWidth: true
                font.pixelSize: units.fx("small")
                text: "<a href='https://www.last.fm/api/account/create'>" + qsTr("Get an API account") + "</a>"
                onLinkActivated: Qt.openUrlExternally(link)
                linkColor: styleMusic.view.linkColor
            }
        }
        TextField {
            id: apiKey
            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: units.fx("medium")
            placeholderText: qsTr("Enter a valid API key");
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
            Layout.fillWidth: true
        }
    }

    SectionHeader { text: qsTr("System") }

    MusicCheckBox {
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("Enable debug log output")
        font.pixelSize: units.fx("medium")
        onClicked: {
            if (checked)
                Sonos.debug(4);
            else
                Sonos.debug(mainView.debugLevel);
        }
    }
}
