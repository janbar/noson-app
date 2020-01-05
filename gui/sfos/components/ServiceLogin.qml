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
import NosonApp 1.0

// Overlay to show when auth expired
Rectangle {
    id: loginLauncher
    anchors.fill: parent
    color: "transparent"

    Column {
        id: noMusicTextColumn
        anchors.centerIn: parent
        spacing: units.gu(3)
        width: parent.width > units.gu(44) ? parent.width - units.gu(8) : units.gu(36)

        Label {
            color: styleMusic.view.labelColor
            elide: Text.ElideRight
            font.pixelSize: units.fx("large")
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 2
            text: qsTr("Registering the service")
            width: parent.width
            wrapMode: Text.WordWrap
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
            font.pixelSize: units.fx("medium")
        }

        Label {
            id: loginOutput
            color: "red"
            visible: false // should only be visible when an error is made.
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 1
            font.weight: Font.Normal
            font.pixelSize: units.fx("medium")
        }

        TextField {
            id: username
            anchors {
                left: parent.left
                right: parent.right
            }
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhEmailCharactersOnly
            placeholderText: qsTr("User name")
            font.pixelSize: units.fx("medium")
        }

        TextField {
            id: password
            anchors {
                left: parent.left
                right: parent.right
            }
            inputMethodHints: Qt.ImhHiddenText
            placeholderText: qsTr("Password")
            echoMode: TextInput.Password
            font.pixelSize: units.fx("medium")

            Keys.onReturnPressed: signInService()
            Keys.onEnterPressed: signInService()
        }

        Component.onCompleted: {
            username.text = mediaModel.username;
        }

        Button {
            id: loginButton
            height: units.gu(6)
            //: this appears in a button with limited space (around 30 characters)
            text: qsTr("Submit")
            width: parent.width

            onClicked: signInService()
        }
    }

    function signInService() {
        mainView.jobRunning = true; // it will be cleared on delayLoginService finished
        delayLoginService.start();
    }

    Timer {
        id: delayLoginService
        interval: 100
        onTriggered: {
            loginOutput.visible = false;
            var ret = mediaModel.requestSessionId(username.text, password.text);
            mainView.jobRunning = false;
            if (ret === 0) {
                customdebug("Service login failed.");
                loginOutput.text = qsTr("Login failed.");
                loginOutput.visible = true;
            }
        }
    }
}
