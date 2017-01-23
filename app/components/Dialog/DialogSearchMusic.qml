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
import Ubuntu.Components.Popups 1.3
import Ubuntu.Components.ListItems 1.3

DialogBase {
    id: dialogSearchMusic
    // TRANSLATORS: this is a title of a dialog to setup search
    title: i18n.tr("Search music")

    property var searchableModel

    ListModel {
        id: selectorModel
    }

    Component.onCompleted: {
        if (searchableModel) {
            var list = searchableModel.listSearchCategories();
            if (list.length) {
                for (var i = 0; i < list.length; ++i) {
                    var id = list[i];
                    var tr;
                    if (id === "artists")
                        tr = i18n.tr("Artists");
                    else if (id === "albums")
                        tr = i18n.tr("Albums");
                    else if (id === "tracks")
                        tr = i18n.tr("Songs");
                    else if (id === "playlists")
                        tr = i18n.tr("Playlists");
                    else if (id === "stations")
                        tr = i18n.tr("Radios");
                    else if (id === "podcasts")
                        tr = i18n.tr("Podcasts");
                    else if (id === "genres")
                        tr = i18n.tr("Genres");
                    else if (id === "composers")
                        tr = i18n.tr("Composers")
                    else
                        tr = id;
                    selectorModel.insert(i, {'id': id, 'text': tr});
                }
                selector.selectedIndex = 0;
            }
        }
    }

    ItemSelector {
        id: selector
        text: ""
        model: selectorModel
        containerHeight: itemHeight * 6
        expanded: false
        multiSelection: false
        selectedIndex: -1

        delegate: OptionSelectorDelegate {
            text: selected ? "<font color=\"white\">" + i18n.tr(model.text) + "</font>"
                           : "<font color=\"black\">" + model.text + "</font>"
        }
    }

    TextField {
        id: searchField
        anchors {
            left: parent.left
            right: parent.right
        }
        color: theme.palette.selected.baseText
        focus: true
        hasClearButton: true
        inputMethodHints: Qt.ImhNoPredictiveText
        placeholderText: i18n.tr("Search music")
        font.pixelSize: 16
    }

    Button {
        text: i18n.tr("Search")
        color: styleMusic.dialog.confirmButtonColor
        onClicked: {
            if (searchableModel !== null && selector.selectedIndex >= 0 && searchField.text.length) {
                searchableModel.loadSearch(selectorModel.get(selector.selectedIndex).id, searchField.text);
            }
            PopupUtils.close(dialogSearchMusic);
        }
    }

    Button {
        text: i18n.tr("Cancel")
        color: styleMusic.dialog.cancelButtonColor
        onClicked: PopupUtils.close(dialogSearchMusic)
    }
}
