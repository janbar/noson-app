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

import QtQuick 2.4
import Ubuntu.Components 1.3
import NosonApp 1.0

// Overlay to show when auth expired
Rectangle {
    id: registrationLauncher
    anchors {
        fill: parent
    }
    color: mainView.backgroundColor

    Column {
        id: noMusicTextColumn
        anchors {
            centerIn: parent
        }
        spacing: units.gu(4)
        width: parent.width > units.gu(44) ? parent.width - units.gu(8) : units.gu(36)

        Label {
            color: styleMusic.libraryEmpty.labelColor
            elide: Text.ElideRight
            fontSize: "large"
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 2
            text: i18n.tr("Registering the service")
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Label {
            id: regCode
            visible: text.length > 0
            color: styleMusic.libraryEmpty.labelColor
            elide: Text.ElideRight
            fontSize: "large" //font.pointSize: 20
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 2
            text: ""
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Label {
            id: regMessage
            color: styleMusic.libraryEmpty.labelColor
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 4
            text: i18n.tr("This will require to authenticate against the music service again, as credentials cannot be retrieved from Sonos device.")
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Label {
            id: regUrl
            visible: text.length > 0
            color: styleMusic.libraryEmpty.labelColor
            elide: Text.ElideRight
            fontSize: "large" //font.pointSize: 20
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 6
            text: ""
            onLinkActivated: Qt.openUrlExternally(link)
            linkColor: UbuntuColors.green
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Button {
            id: regStartButton
            color: UbuntuColors.green
            height: units.gu(4)
            // TRANSLATORS: this appears in a button with limited space (around 30 characters)
            text: i18n.tr("Start service registration")
            width: parent.width

            onClicked: {
                mainView.jobRunning = true
                delayRegisterService.start()
            }
        }

        Timer {
            id: delayRegisterService
            interval: 100
            onTriggered: {
                if (mediaModel.beginDeviceRegistration()) {
                    regStartButton.visible = false;
                    regCode.text = mediaModel.linkCode;
                    regMessage.text = i18n.tr("Click the link below to authorize this application to use the service.")
                    regUrl.text = "<a href='" + mediaModel.regURL + "'>" + mediaModel.regURL + "</a>";
                    requestAuthForTime.start();
                }
                 mainView.jobRunning = false
            }
        }

        Timer {
            id: requestAuthForTime
            triggeredOnStart: true
            interval: 3000
            repeat: true
            onTriggered: {
                var ret = mediaModel.requestDeviceAuth();
                if (ret) {
                    stop();
                } else {
                    customdebug("Retry request auth ...");
                }
            }
        }
    }
}
