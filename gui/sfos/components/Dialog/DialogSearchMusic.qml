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
import QtQml.Models 2.2

DialogBase {
    id: dialog

    property var searchableModel

    acceptText: qsTr("Search")
    canAccept: searchField.text.length > 0

    ComboBox {
        id: selector
        currentIndex: -1
        
        menu: ContextMenu {
            id: contextMenu
        }
    }

    TextField {
        id: searchField
        anchors.left: parent.left
        anchors.right: parent.right
        placeholderText: qsTr("Type search")
        font.pixelSize: units.fx("medium")
    }
    
    ListModel {
        id: selectorModel
    }

    Component {
        id: menuItemComp
        MenuItem {}
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
                    var newMenuItem = menuItemComp.createObject(contextMenu._contentColumn, {"id": id, "text" : tr})
                }
                selector.currentIndex = 0;
            }
        }
    }

    onAccepted: {
        if (searchableModel !== null && selector.currentIndex >= 0 && searchField.text.length) {
            searchableModel.asyncLoadSearch(selectorModel.get(selector.currentIndex).id, searchField.text);
        }
        dialog.close();
    }
}
