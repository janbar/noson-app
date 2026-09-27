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

import QtQuick 2.9
import QtQuick.Controls 2.2

MusicListView {

    property bool hasSelection: false

    state: "default"
    states: [
        State { name: "default" },
        State { name: "selection" }
    ]

    property var selectedIndices: []
    property int selectedCount: 0 // selectedIndices is mutated in place, so bindings need this

    signal synchronizeChecked() // need item sync for CheckBox state
    signal selectNone()
    signal selectAll()

    signal selected(int index)
    signal deselected(int index)

    onSelectNone: {
        selectedIndices = []
        synchronizeChecked()
        //selectMode = false;
        //selectMode = true;
        //hasSelection = false;
    }
    onSelectAll: {
        var tmp = []
        for (var i = 0; i < model.count; ++i) {
            tmp.push(i)
        }
        selectedIndices = tmp
        synchronizeChecked()
        //selectMode = false;
        //selectMode = true;
        //hasSelection = (tmp.length > 0);
    }
    onSynchronizeChecked: {
        selectedCount = selectedIndices.length
        if (selectedCount > 0)
            hasSelection = true;
    }

    // Expose the selectedIndices
    function getSelectedIndices() {
        var indicies = selectedIndices.slice();
        indicies.sort();  // ensure indicies are in-order
        return indicies;
    }

    function isSelectedIndex(i) {
        return selectedIndices.indexOf(i) >= 0
    }

    function selectIndex(i) {
        selectedIndices.push(i)
        selectedCount = selectedIndices.length
        if (!hasSelection)
            hasSelection = true;
        selected(i);
    }

    function deselectIndex(i) {
        var index = selectedIndices.indexOf(i)
        selectedIndices.splice(index, 1)
        selectedCount = selectedIndices.length
        if (selectedCount === 0)
            hasSelection = false;
        deselected(i);
    }
}
