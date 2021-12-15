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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQml.Models 2.3

DialogBase {
    id: dialog

    property var searchableModel
    property string searchType

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
            text: qsTr("Search")
            onClicked: dialog.accept()
        }
    }

    Label {
        //: this is a title of a dialog to setup search
        text: qsTr("Search music")
        color: styleMusic.dialog.labelColor
        font.pointSize: units.fs("large")
        font.bold: true
    }

    ListModel {
        id: selectorModel
    }

    onOpened: {
        _search = false; // fix qt5.9
        if (selector.currentIndex < 0 && searchableModel !== null) {
            var list = searchableModel.listSearchCategories();
            if (list.length) {
                for (var i = 0; i < list.length; ++i) {
                    var id = list[i];
                    var tt = id.split("::", 2);
                    var tr = "";
                    if (tt.length > 1)
                        tr = tt[1] + " / ";
                    if (tt[0] === "artists")
                        tr += qsTr("Artists");
                    else if (tt[0] === "albums")
                        tr += qsTr("Albums");
                    else if (tt[0] === "tracks")
                        tr += qsTr("Tracks");
                    else if (tt[0] === "playlists")
                        tr += qsTr("Playlists");
                    else if (tt[0] === "stations")
                        tr += qsTr("Radios");
                    else if (tt[0] === "podcasts")
                        tr += qsTr("Podcasts");
                    else if (tt[0] === "genres")
                        tr += qsTr("Genres");
                    else if (tt[0] === "composers")
                        tr += qsTr("Composers")
                    else
                        tr += tt[0];
                    selectorModel.insert(i, {'id': id, 'text': tr});
                    if (searchType && id === searchType)
                        selector.currentIndex = i;
                }
                if (selector.currentIndex < 0 && selectorModel.count > 0)
                    selector.currentIndex = 0;
            }
        }
    }

    ComboBox {
        id: selector
        textRole: "text"
        model: selectorModel
        Layout.fillWidth: true
        font.pointSize: units.fs("medium")
        currentIndex: -1
        Component.onCompleted: {
            popup.font.pointSize = font.pointSize;
        }
    }

    TextField {
        id: searchField
        anchors {
            left: parent.left
            right: parent.right
        }
        focus: true
        inputMethodHints: Qt.ImhNoPredictiveText
        placeholderText: qsTr("Type search")
        font.pointSize: units.fs("medium")
        Keys.onReturnPressed: dialog.accept()
    }

    onAccepted: {
        _search = true; // fix qt5.9
        if (searchableModel !== null && selector.currentIndex >= 0 && searchField.text.length) {
            searchableModel.asyncLoadSearch(selectorModel.get(selector.currentIndex).id, searchField.text);
        }
        dialog.close();
    }

    property bool _search: false    // fix qt5.9: dialog result not defined
}
