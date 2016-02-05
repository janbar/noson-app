/*
 * Copyright (C) 2013, 2014, 2015
 *      Andrew Hayzen <ahayzen@gmail.com>
 *      Daniel Holm <d.holmen@gmail.com>
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

MusicListView {
    // Can't access ViewItems externally
    // so we need to expose if in multiselect mode for the header states
    state: ViewItems.selectMode ? "multiselectable" : "normal"

    signal clearSelection()
    signal closeSelection()
    signal reorder(int from, int to)
    signal selectAll()

    onClearSelection: {
        ViewItems.selectedIndices = []
        ViewItems.selectMode = false
        ViewItems.selectMode = true
    }
    onCloseSelection: {
        ViewItems.selectedIndices = []
        ViewItems.selectMode = false
        ViewItems.dragMode = false
    }
    onSelectAll: {
        var tmp = []

        for (var i=0; i < model.count; i++) {
            tmp.push(i)
        }

        ViewItems.selectedIndices = tmp
    }

    // Can't access ViewItems externally
    // so for the header actions we need to expose the selectedIndices
    function getSelectedIndices() {
        var indicies = ViewItems.selectedIndices.slice();

        indicies.sort();  // ensure indicies are in-order

        return indicies;
    }

    ViewItems.selectMode: false
    ViewItems.dragMode: false
    ViewItems.onDragUpdated: {
        // Only update the model when the listitem is dropped, not 'live'
        if (event.status == ListItemDrag.Moving) {
            event.accept = false
        } else if (event.status == ListItemDrag.Dropped) {
            //model.move(event.from, event.to, 1);
            reorder(event.from, event.to)
        }
    }
}
