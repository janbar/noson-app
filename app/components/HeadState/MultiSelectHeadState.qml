/*
 * Copyright (C) 2015, 2016
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Andrew Hayzen <ahayzen@gmail.com>
 *      Victor Thompson <victor.thompson@gmail.com>
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
import Ubuntu.Components 1.2
import "../Flickables"

PageHeadState {
    id: selectionState
    actions: [
        Action {
            enabled: listview !== null ?
                         (listview.model.count > 0) || listview.getSelectedIndices().length > 0 : false
            iconName: "select"
            text: i18n.tr("Select All")

            onTriggered: {
                if (listview.getSelectedIndices().length > 0) {
                    listview.clearSelection()
                } else {
                    listview.selectAll()
                }
            }
        },
        Action {
            enabled: listview !== null ? listview.getSelectedIndices().length > 0 : false
            iconName: "add"
            text: i18n.tr("Add to queue")
            visible: addToQueue

            onTriggered: {
                var indicies = listview.getSelectedIndices();
                // when select all then add the container if exists
                if (containerItem && indicies.length === listview.model.count) {
                    if (addQueue(containerItem))
                        listview.closeSelection();
                }
                else {
                    var items = [];
                    for (var i = 0; i < indicies.length; i++) {
                        items.push(listview.model.get(indicies[i]));
                    }
                    if (addMultipleItemsToQueue(items))
                        listview.closeSelection();
                }
            }
        },
        Action {
            enabled: listview !== null ? listview.getSelectedIndices().length > 0 : false
            iconName: "add-to-playlist"
            text: i18n.tr("Add to playlist")
            visible: addToPlaylist

            onTriggered: {
                var items = []
                var indicies = listview.getSelectedIndices();
                // when select all then add the container if exists
                if (containerItem && indicies.length === listview.model.count)
                    items.push(containerItem);
                else {
                    for (var i = 0; i < indicies.length; i++) {
                        items.push(listview.model.get(indicies[i]));
                    }
                }
                mainPageStack.push(Qt.resolvedUrl("../../ui/AddToPlaylist.qml"),
                                   {"chosenElements": items})
                listview.closeSelection()
            }
        },
        Action {
            enabled: listview !== null ? listview.getSelectedIndices().length > 0 : false
            iconName: "delete"
            text: i18n.tr("Delete")
            visible: removable

            onTriggered: {
                removed(listview.getSelectedIndices())
                listview.closeSelection()
            }
        }

    ]
    backAction: Action {
        text: i18n.tr("Cancel selection")
        iconName: "back"
        onTriggered: listview.closeSelection()
    }
    head: thisPage.head
    name: "selection"

    PropertyChanges {
        target: thisPage.head
        backAction: selectionState.backAction
        actions: selectionState.actions
    }

    property var containerItem

    property bool addToQueue: true
    property bool addToPlaylist: true
    property MultiSelectListView listview
    property bool removable: false
    property Page thisPage

    signal removed(var selectedIndices)
}
