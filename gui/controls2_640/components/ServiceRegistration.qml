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
import NosonApp 1.0

// Overlay to show when auth expired
Rectangle {
    id: registrationLauncher
    anchors {
        fill: parent
    }
    color: styleMusic.view.backgroundColor

    Column {
        id: noMusicTextColumn
        anchors {
            centerIn: parent
        }
        spacing: units.gu(4)
        width: parent.width > units.gu(44) ? parent.width - units.gu(8) : units.gu(36)

        Label {
            color: styleMusic.view.labelColor
            elide: Text.ElideRight
            font.pointSize: units.fs("large")
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 2
            text: qsTr("Registering the service")
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Label {
            id: regCode
            visible: text.length > 0
            color: styleMusic.view.foregroundColor
            elide: Text.ElideRight
            font.pointSize: units.fs("large")
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 2
            text: ""
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Button {
            id: openLink
            visible: false
            height: units.gu(6)
            text: qsTr("Open the registration link")
            width: parent.width
            onClicked: Qt.openUrlExternally(mediaModel.regURL)
        }

        Label {
            id: regMessage
            color: styleMusic.view.foregroundColor
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 10
            text: qsTr("First, the third-party service must have been configured with the official Sonos app. Then this will require authenticating again with the music service, as the credentials cannot be retrieved from the Sonos device.")
            width: parent.width
            wrapMode: Text.WordWrap
            font.pointSize: units.fs("medium")
        }

        TextInput {
            id: regUrl
            visible: openLink.visible
            color: styleMusic.view.highlightedColor
            readOnly: true
            selectByMouse: true
            font.pointSize: units.fs("medium")
            horizontalAlignment: Text.AlignHCenter
            text: ""
            width: parent.width
            wrapMode: Text.WrapAnywhere
        }

        Button {
            id: regStartButton
            height: units.gu(6)
            //: this appears in a button with limited space (around 30 characters)
            text: qsTr("Start service registration")
            width: parent.width

            onClicked: {
                mainView.jobRunning = true // it will be cleared on delayRegisterService finished
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
                    regMessage.text = qsTr("Or copy the link below and paste it into your browser ...")
                    regUrl.text = mediaModel.regURL;
                    openLink.visible = true;
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
