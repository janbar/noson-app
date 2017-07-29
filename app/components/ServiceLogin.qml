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
    id: loginLauncher
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
            id: regMessage
            color: styleMusic.libraryEmpty.labelColor
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 6
            text: i18n.tr("This will require to authenticate against the music service again, as credentials cannot be retrieved from Sonos device.")
            width: parent.width
            wrapMode: Text.WordWrap
        }

        Label {
            id: loginOutput
            color: UbuntuColors.red
            visible: false // should only be visible when an error is made.
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            maximumLineCount: 1
            font.weight: Font.Normal
        }

        TextField {
            id: username
            anchors {
                left: parent.left
                right: parent.right
            }
            color: theme.palette.selected.baseText
            focus: true
            hasClearButton: true
            inputMethodHints: Qt.ImhNoPredictiveText
            placeholderText: i18n.tr("User name")
        }

        TextField {
            id: password
            anchors {
                left: parent.left
                right: parent.right
            }
            color: theme.palette.selected.baseText
            focus: true
            hasClearButton: true
            inputMethodHints: Qt.ImhNoPredictiveText
            placeholderText: i18n.tr("Password")
            echoMode: TextInput.Password
        }

        Component.onCompleted: {
           username.text = mediaModel.username;
        }

        Button {
            id: loginButton
            color: UbuntuColors.green
            height: units.gu(4)
            // TRANSLATORS: this appears in a button with limited space (around 30 characters)
            text: i18n.tr("Submit")
            width: parent.width

            onClicked: {
                mainView.currentlyWorking = true
                delayLoginService.start()
            }
        }

        Timer {
            id: delayLoginService
            interval: 100
            onTriggered: {
                loginOutput.visible = false;
                var ret = mediaModel.requestSessionId(username.text, password.text);
                mainView.currentlyWorking = false;
                if (ret === 0) {
                    customdebug("Service login failed.");
                    loginOutput.text = i18n.tr("Login failed.");
                    loginOutput.visible = true;
                }
            }
        }

    }
}
