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
import QtQuick.Layouts 1.1
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import NosonThumbnailer 1.0

Page {
    id: noZoneStatePage

    property string pageTitle: qsTr("No zone")
    property bool isRoot: true

    backNavigation: false

    // Overlay to show when no zone found
    Rectangle {
        id: scanLauncher
        anchors {
            fill: parent
        }
        color: styleMusic.view.backgroundColor

        Row {
            id: images
            anchors {
                horizontalCenter: noMusicTextColumn.horizontalCenter
                bottom: noMusicTextColumn.top
                bottomMargin: units.gu(10)
            }

            Item {
                height: parent.height
                width: imageEmptyDownload.width + units.gu(2)
                anchors.right: sep.left

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
                    source: "qrc:/images/music_download_icon.png"
                }

                Rectangle {
                    id: imageEmptyDownloadFill
                    visible: false
                    anchors.fill: imageEmptyDownload
                    color: "darkgray"
                }

                OpacityMask {
                    anchors.fill: imageEmptyDownloadFill
                    source: imageEmptyDownloadFill
                    maskSource: imageEmptyDownload
                }
            }

            Item {
                id: sep
                height: parent.height
                width: units.gu(7)
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    id: imageSep
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                    antialiasing: true
                    //fillMode: Image.PreserveAspectFit
                    width: 1
                    height: units.gu(6)
                    smooth: true
                    source: "qrc:/images/div.png"
                }

                Rectangle {
                    id: imageSepFill
                    visible: false
                    anchors.fill: imageSep
                    color: "darkgray"
                }

                OpacityMask {
                    anchors.fill: imageSepFill
                    source: imageSepFill
                    maskSource: imageSep
                }
            }

            Item {
                height: parent.height
                width: imageEmptySD.width + units.gu(2)
                anchors.left: sep.right

                Image {
                    id: imageEmptySD
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                    antialiasing: true
                    fillMode: Image.PreserveAspectFit
                    height: units.gu(7)
                    smooth: true
                    source: "qrc:/images/wifi_phone_icon.png"
                }

                Rectangle {
                    id: imageEmptySDFill
                    visible: false
                    anchors.fill: imageEmptySD
                    color: "darkgray"
                }

                OpacityMask {
                    anchors.fill: imageEmptySDFill
                    source: imageEmptySDFill
                    maskSource: imageEmptySD
                }
            }
        }

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
                font.pointSize: 22.0
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 2
                text: qsTr("No Sonos zone found")
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Text {
                color: styleMusic.view.foregroundColor
                elide: Text.ElideRight
                font.pixelSize: units.fx("medium")
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 6
                text: qsTr("Make sure that your device is connected to the correct wireless network\nand one or more Sonos products are receiving power.")
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Button {
                //background: green
                height: units.gu(5)
                width: units.gu(36)
                anchors.horizontalCenter: parent.horizontalCenter
                //: this appears in a button with limited space (around 30 characters)
                text: qsTr("Search for Sonos zones")
                enabled: !mainView.jobRunning
                onClicked: {
                    Thumbnailer.reset(); // reset thumbnailer state
                    connectSonos();
                }
            }
        }
    }
}
