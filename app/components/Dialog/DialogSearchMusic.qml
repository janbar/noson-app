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
    id: dialogSearchMusic
    standardButtons: Dialog.Ok | Dialog.Cancel

    property var searchableModel

    Label {
        //: this is a title of a dialog to setup search
        text: qsTr("Search music")
        font.pointSize: units.fs("large")
        font.bold: true
    }

    ListModel {
        id: selectorModel
    }

    onOpened: {
        if (selector.currentIndex < 0 && searchableModel !== null) {
            var list = searchableModel.listSearchCategories();
            if (list.length) {
                for (var i = 0; i < list.length; ++i) {
                    var id = list[i];
                    var tr;
                    if (id === "artists")
                        tr = qsTr("Artists");
                    else if (id === "albums")
                        tr = qsTr("Albums");
                    else if (id === "tracks")
                        tr = qsTr("Songs");
                    else if (id === "playlists")
                        tr = qsTr("Playlists");
                    else if (id === "stations")
                        tr = qsTr("Radios");
                    else if (id === "podcasts")
                        tr = qsTr("Podcasts");
                    else if (id === "genres")
                        tr = qsTr("Genres");
                    else if (id === "composers")
                        tr = qsTr("Composers")
                    else
                        tr = id;
                    selectorModel.insert(i, {'id': id, 'text': tr});
                }
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
        color: styleMusic.mainView.normalTextBaseColor
        focus: true
        inputMethodHints: Qt.ImhNoPredictiveText
        placeholderText: qsTr("Type search")
        font.pointSize: units.fs("medium")
    }

    onAccepted: {
        if (searchableModel !== null && selector.currentIndex >= 0 && searchField.text.length) {
            searchableModel.asyncLoadSearch(selectorModel.get(selector.currentIndex).id, searchField.text);
        }
        dialogSearchMusic.close();
    }
}
