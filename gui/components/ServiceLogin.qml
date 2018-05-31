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
    id: loginLauncher
    anchors.fill: parent
    color: styleMusic.view.backgroundColor

    Column {
        id: noMusicTextColumn
        anchors.centerIn: parent
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
            id: regMessage
            color: styleMusic.view.foregroundColor
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 6
            text: qsTr("This will require to authenticate against the music service again, as credentials cannot be retrieved from Sonos device.")
            width: parent.width
            wrapMode: Text.WordWrap
            font.pointSize: units.fs("medium")
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
            font.pointSize: units.fs("medium")
        }

        TextField {
            id: username
            anchors {
                left: parent.left
                right: parent.right
            }
            focus: true
            inputMethodHints: Qt.ImhNoPredictiveText
            placeholderText: qsTr("User name")
            font.pointSize: units.fs("medium")
        }

        TextField {
            id: password
            anchors {
                left: parent.left
                right: parent.right
            }
            focus: true
            inputMethodHints: Qt.ImhNoPredictiveText
            placeholderText: qsTr("Password")
            echoMode: TextInput.Password
            font.pointSize: units.fs("medium")
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

            onClicked: {
                mainView.jobRunning = true
                delayLoginService.start()
            }
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
}
