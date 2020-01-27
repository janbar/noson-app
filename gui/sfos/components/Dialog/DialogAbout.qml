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

DialogBase {
    id: dialog
    title: qsTr("About")

    cancelText: qsTr("Close")
    acceptText: ""
    canAccept: false

    contentSpacing: units.gu(2)

    Text {
        color: styleMusic.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("The project has started in 2015 and is intented to make a fast and smart controller for your SONOS devices."
                   + " You can browse your music library and play track or radio on any zones."
                   + " You can manage grouping zones, queue, and playlists, and fully control the playback.")
        wrapMode: Label.Wrap
        font.pixelSize: units.fx("medium")
    }
    Text {
        color: styleMusic.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("Author: %1").arg("Jean-Luc Barriere")
        font.pixelSize: units.fx("medium")
    }
    Text {
        color: styleMusic.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("Version: %1").arg(VersionString) + " (libnoson " + Sonos.getLibVersion() + ")"
        font.pixelSize: units.fx("medium")
    }
    Text {
        id: donate
        color: styleMusic.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        font.pixelSize: units.fx("small")
        text: "<a href='https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=jlbarriere68%40gmail%2ecom&lc=US&item_name=noson%2dapp&currency_code=EUR&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted'>Donate with Paypal</a>"
        onLinkActivated: Qt.openUrlExternally(link)
        linkColor: styleMusic.view.linkColor
    }

    Text {
        color: styleMusic.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("Sailfish port by: %1").arg("Adam Pigg")
        font.pixelSize: units.fx("medium")
    }
    Text {
        id: donate2
        color: styleMusic.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        font.pixelSize: units.fx("small")
        text: "<a href='https://www.paypal.me/piggz'>Donate with Paypal</a>"
        onLinkActivated: Qt.openUrlExternally(link)
        linkColor: styleMusic.view.linkColor
    }

    Text {
        color: styleMusic.dialog.foregroundColor
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Theme.horizontalPageMargin
            rightMargin: Theme.horizontalPageMargin
        }
        text: qsTr("Thumbnails powered by:")
        font.pixelSize: units.fx("medium")
    }
    Column {
        width: parent.width
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
