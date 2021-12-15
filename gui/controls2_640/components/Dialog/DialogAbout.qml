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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import NosonApp 1.0
import NosonThumbnailer 1.0


DialogBase {
    id: dialog
    title: qsTr("About")

    footer: Row {
        leftPadding: units.gu(1)
        rightPadding: units.gu(1)
        bottomPadding: units.gu(1)
        spacing: units.gu(1)
        layoutDirection: Qt.RightToLeft

        Button {
            flat: true
            text: qsTr("Close")
            onClicked: dialog.reject()
        }
    }

    contentSpacing: units.gu(1)

    Text {
        color: styleMusic.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("This project was started in 2015 with the intention of creating a fast, smart controller for your SONOS devices."
                   + " Browse and manage your music libraries by groupings and play to any zone. Listen to radio stations from around"
                   + " the world and queue up your tracks just the way you like it. Enjoy!")
        wrapMode: Label.Wrap
        font.pointSize: units.fs("medium")
    }
    Text {
        color: styleMusic.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("Author: %1").arg("Jean-Luc Barriere")
        font.pointSize: units.fs("medium")
    }
    Text {
        color: styleMusic.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("Version: %1").arg(VersionString) + " (libnoson " + Sonos.getLibVersion() + ")"
        font.pointSize: units.fs("medium")
    }
    Text {
        id: donate
        color: styleMusic.dialog.foregroundColor
        width: dialog.availableWidth
        font.pointSize: units.fs("medium")
        text: "<a href='https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=jlbarriere68%40gmail%2ecom&lc=US&item_name=noson%2dapp&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted'>Donate with Paypal</a>"
        onLinkHovered: {
            if (hoveredLink)
                font.bold = true;
            else
                font.bold = false;
        }
        onLinkActivated: Qt.openUrlExternally(link)
        linkColor: styleMusic.view.linkColor
    }

    Item {
        width: dialog.availableWidth
        height: units.gu(0.5)
    }

    Text {
        color: styleMusic.dialog.foregroundColor
        width: dialog.availableWidth
        text: qsTr("Thumbnails powered by:")
        font.pointSize: units.fs("medium")
    }
    Column {
        width: dialog.availableWidth
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            color: "transparent"
            width: units.gu(10)
            height: units.gu(3)
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (lastfm.visible)
                        Qt.openUrlExternally("https://www.last.fm");
                    else if (deezer.visible)
                        Qt.openUrlExternally("https://www.deezer.com");
                }
            }

            Image {
                id: lastfm
                width: parent.width
                fillMode: Image.PreserveAspectCrop
                source: "qrc:/images/lastfm.png"
                sourceSize.width: width
                visible: (thumbApiName === "LASTFM")
            }

            Image {
                id: deezer
                width: parent.width
                fillMode: Image.PreserveAspectCrop
                source: settings.theme === 1 ? "qrc:/images/Deezer_Logo_RVB_White.svg" : "qrc:/images/Deezer_Logo_RVB_Black.svg"
                sourceSize.width: width
                visible: (thumbApiName === "DEEZER")
            }
        }
    }

    property string thumbApiName: ""

    onOpened: {
        thumbApiName = Thumbnailer.apiName();
    }
}
