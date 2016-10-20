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

Page {
    id: noZoneStatePage
    anchors {
        fill: parent
    }

    // Overlay to show when no zone found
    Rectangle {
        id: scanLauncher
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

            Row {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

                Item {
                    height: parent.height
                    width: imageEmptyDownload.width + units.gu(2)

                    Image {
                        id: imageEmptyDownload
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            verticalCenter: parent.verticalCenter
                        }
                        antialiasing: true
                        fillMode: Image.PreserveAspectFit
                        height: units.gu(10)
                        smooth: true
                        source: "../graphics/music_download_icon.png"
                    }
                }

                Item {
                    height: parent.height
                    width: units.gu(7)

                    Image {
                        id: imageSep
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            verticalCenter: parent.verticalCenter
                        }
                        antialiasing: true
                        fillMode: Image.PreserveAspectFit
                        height: units.gu(6)
                        smooth: true
                        source: "../graphics/div.png"
                    }
                }

                Image {
                    id: imageEmptySD
                    anchors {
                        verticalCenter: parent.verticalCenter
                    }
                    antialiasing: true
                    fillMode: Image.PreserveAspectFit
                    height: units.gu(7)
                    smooth: true
                    source: "../graphics/wifi_phone_icon.png"
                }
            }

            Label {
                color: styleMusic.libraryEmpty.labelColor
                elide: Text.ElideRight
                fontSize: "x-large"
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 2
                text: i18n.tr("No Sonos zone found")
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Label {
                color: styleMusic.libraryEmpty.labelColor
                elide: Text.ElideRight
                fontSize: "large"
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 6
                text: i18n.tr("Make sure that your device is connected to the correct wireless network and one or more Sonos products are receiving power.")
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Button {
                color: UbuntuColors.green
                height: units.gu(4)
                // TRANSLATORS: this appears in a button with limited space (around 30 characters)
                text: i18n.tr("Search for Sonos zones")
                width: parent.width

                onClicked: {
                    mainView.currentlyWorking = true
                    delayConnectSonos.start()
                }

                Timer {
                    id: delayConnectSonos
                    interval: 100
                    onTriggered: {
                        connectSonos()
                        mainView.currentlyWorking = false
                    }
                }
            }
        }
    }
}
