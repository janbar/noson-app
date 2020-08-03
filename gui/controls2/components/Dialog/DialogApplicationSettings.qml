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
import NosonThumbnailer 1.0
import NosonMediaScanner 1.0
import "../"


DialogBase {
    id: dialog
    title: qsTr("General settings")

    footer: Row {
        leftPadding: units.gu(1)
        rightPadding: units.gu(1)
        bottomPadding: units.gu(1)
        spacing: units.gu(1)
        layoutDirection: Qt.RightToLeft

        Button {
            flat: true
            text: qsTr("Cancel")
            onClicked: dialog.reject()
        }
        Button {
            flat: true
            text: qsTr("Ok")
            onClicked: dialog.accept()
        }
    }

    onOpened: {
        fontScaleBox.acceptedValue = settings.fontScaleFactor;
        scaleBox.acceptedValue = settings.scaleFactor;
        themeBox.acceptedValue = settings.theme;
        apiKey.text = settings.lastfmKey;
        addMusicPath.text = settings.musicLocation;
    }
    onAccepted: {
        var needRestart = (styleBox.currentIndex !== styleBox.styleIndex ||
                scaleBox.realValue !== scaleBox.acceptedValue);

        settings.style = styleBox.displayText;
        scaleBox.acceptedValue = settings.scaleFactor;

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

        var mdir = addMusicPath.displayText.trim();
        if (settings.musicLocation !== mdir) {
            if (settings.musicLocation.length > 0)
                MediaScanner.removeRootPath(settings.musicLocation);
            if (mdir.length > 0 && MediaScanner.addRootPath(mdir))
                settings.musicLocation = mdir;
            else
                settings.musicLocation = "";
        }

        if (needRestart) {
            mainView.jobRunning = true;
            Qt.exit(16);
        }
    }
    onRejected: {
        styleBox.currentIndex = styleBox.styleIndex
        mainView.width = Math.round(scaleBox.acceptedValue * mainView.width / settings.scaleFactor);
        mainView.height = Math.round(scaleBox.acceptedValue * mainView.height / settings.scaleFactor);
        settings.fontScaleFactor = fontScaleBox.acceptedValue;
        settings.scaleFactor = scaleBox.acceptedValue;
        settings.theme = themeBox.acceptedValue;
        apiKey.text = settings.lastfmKey;
    }

    ColumnLayout {
        id: settingsColumn
        spacing: units.gu(1)

        RowLayout {
            spacing: 0
            Icon {
                height: units.gu(5)
                width: height
                source: "qrc:/images/font-scalling.svg"
                hoverEnabled: false
            }
            SpinBox {
                id: fontScaleBox
                enabled: !Android
                from: 50
                value: settings.fontScaleFactor * 100
                to: 200
                stepSize: 10
                font.pointSize: units.fs("medium");
                Layout.fillWidth: true

                property int decimals: 2
                property real realValue: value / 100
                property real acceptedValue: 0

                validator: DoubleValidator {
                    bottom: Math.min(fontScaleBox.from, fontScaleBox.to)
                    top:  Math.max(fontScaleBox.from, fontScaleBox.to)
                }

                textFromValue: function(value, locale) {
                    return Number(value / 100).toLocaleString(locale, 'f', fontScaleBox.decimals)
                }

                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text) * 100
                }

                onValueModified: {
                    settings.fontScaleFactor = realValue
                }
            }
        }

        RowLayout {
            spacing: 0
            Icon {
                height: units.gu(5)
                width: height
                source: "qrc:/images/graphic-scalling.svg"
                hoverEnabled: false
            }
            SpinBox {
                id: scaleBox
                enabled: !Android
                from: 50
                value: settings.scaleFactor * 100
                to: 400
                stepSize: 10
                font.pointSize: units.fs("medium");
                Layout.fillWidth: true

                property int decimals: 2
                property real realValue: value / 100
                property real acceptedValue: 0

                validator: DoubleValidator {
                    bottom: Math.min(scaleBox.from, scaleBox.to)
                    top:  Math.max(scaleBox.from, scaleBox.to)
                }

                textFromValue: function(value, locale) {
                    return Number(value / 100).toLocaleString(locale, 'f', scaleBox.decimals)
                }

                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text) * 100
                }

                onValueModified: {
                    mainView.width = Math.round(realValue * mainView.width / settings.scaleFactor);
                    mainView.height = Math.round(realValue * mainView.height / settings.scaleFactor);
                    settings.scaleFactor = realValue
                }
            }
        }

        RowLayout {
            spacing: units.gu(1)
            Layout.fillWidth: true
            Label {
                text: qsTr("Style")
                font.pointSize: units.fs("medium");
            }
            ComboBox {
                id: styleBox
                property int styleIndex: -1
                model: AvailableStyles
                Component.onCompleted: {
                    styleIndex = find(settings.style, Qt.MatchFixedString)
                    if (styleIndex !== -1)
                        currentIndex = styleIndex
                }
                onActivated: {
                    // reset theme when not supported
                    if (currentText !== "Material" && currentText !== "Universal") {
                        settings.theme = 0;
                    }
                }
                Layout.fillWidth: true
                font.pointSize: units.fs("medium");
                popup {
                    font.pointSize: units.fs("medium");
                }
            }
        }

        RowLayout {
            spacing: units.gu(1)
            Layout.fillWidth: true
            Label {
                text: qsTr("Perceived loudness volume control")
                font.pointSize: units.fs("medium");
            }

            Text {
                text: "?"

                MouseArea {
                    anchors.fill: parent
                    onClicked: explanation.open()
                }
                font.bold: true
                color: styleMusic.view.linkColor

                Popup {
                    id: explanation
                    Text {
                        textFormat: Text.RichText
                        text: qsTr("Instead of linear 1-100% volume controls, adjust volume by perceived loudness.<br/>For a longer explanation, click <a href=https://www.dr-lex.be/info-stuff/volumecontrols.html>here.</a>")
                        onLinkActivated: Qt.openUrlExternally(link)
                        color: styleMusic.view.labelColor
                        linkColor: styleMusic.view.linkColor
                    }
                }
            }

            CheckBox {
                checked: settings.loudnessVolumeControl
                onCheckedChanged: {
                    settings.loudnessVolumeControl = checked
                }
            }

        }

        RowLayout {
            visible: styleBox.currentText === "Material" || styleBox.currentText === "Universal"
            spacing: units.gu(1)
            Layout.fillWidth: true
            Label {
                text: qsTr("Theme")
                font.pointSize: units.fs("medium");
            }
            ComboBox {
                id: themeBox
                property int acceptedValue: 0
                model: [
                    qsTr("Light"),
                    qsTr("Dark")
                ]

                currentIndex: settings.theme
                onActivated: {
                    settings.theme = index
                }

                Layout.fillWidth: true
                font.pointSize: units.fs("medium");
                Component.onCompleted: {
                    popup.font.pointSize = units.fs("medium");
                }
            }
        }

        Label {
            text: qsTr("Restart is required")
            font.pointSize: units.fs("medium")
            color: "red"
            opacity: styleBox.currentIndex !== styleBox.styleIndex ||
                     scaleBox.realValue !== scaleBox.acceptedValue ? 1.0 : 0.0
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        ColumnLayout {
            visible: !Android
            spacing: units.gu(0.5)
            Layout.fillWidth: true
            Label {
                font.pointSize: units.fs("medium")
                text: qsTr("Additional music location")
            }
            TextField {
                id: addMusicPath
                font.pointSize: units.fs("medium")
                placeholderText: qsTr("Enter a valid path");
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
                EnterKey.type: Qt.EnterKeyDone
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            visible: true
            spacing: units.gu(0.5)
            Layout.fillWidth: true
            RowLayout {
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
                    font.pointSize: units.fs("x-small")
                    text: "<a href='https://www.last.fm/api/account/create'>" + qsTr("Get an API account") + "</a>"
                    onLinkHovered: {
                        if (hoveredLink)
                            font.bold = true;
                        else
                            font.bold = false;
                    }
                    onLinkActivated: Qt.openUrlExternally(link)
                    linkColor: styleMusic.view.linkColor
                }
            }
            TextField {
                id: apiKey
                font.pointSize: units.fs("medium")
                placeholderText: qsTr("Enter a valid API key");
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
                EnterKey.type: Qt.EnterKeyDone
                Layout.fillWidth: true
            }
        }
    }
}
