/*
 * Copyright (C) 2013, 2014, 2015, 2016
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
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
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3

// generic page for music, could be useful for bottomedge implementation
Page {
    id: thisPage
    anchors {
        bottomMargin: musicToolbar.visible ? musicToolbar.height : 0
        fill: parent
    }

    property Dialog currentDialog
    property bool searchable: false
    property int searchResultsCount
    property bool showToolbar: true

    Label {
        anchors {
            centerIn: parent
        }
        text: i18n.tr("No items found")
        visible: parent.state === "search" && searchResultsCount === 0
    }

    // FIXME: hack is a workaround for SDK bug pad.lv/1341814
    // which causes the header and contents of the page to become out of sync
    property Item __oldContents: null

    Connections {
        target: thisPage.head
        onContentsChanged: {
            if (thisPage.__oldContents) {
                thisPage.__oldContents.parent = null;
            }
            thisPage.__oldContents = thisPage.head.contents;
        }
    }

    onVisibleChanged: {
        if (visible) {
            mainPageStack.setPage(thisPage);
        }
    }

    PageHeadSections {
        id: defaultStateSections
        model: [currentZone]
        selectedIndex: -1
    }

    head {
        sections {
            model: defaultStateSections.model
            selectedIndex: defaultStateSections.selectedIndex
            onSelectedIndexChanged: {
                if (head.sections.selectedIndex == 0 && mainPageStack.currentPage.title !== i18n.tr("Zones")) {
                    mainPageStack.push(zonesPageLoader.item), {}
                    head.sections.selectedIndex = -1
                }
            }
        }
    }
}
