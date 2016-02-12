/*
 * Copyright (C) 2015
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
import Ubuntu.Components 1.3

PageHeadState {
    id: headerState
    name: "search"
    head: thisPage.head
    backAction: Action {
        id: leaveSearchAction
        text: "back"
        iconName: "back"
        onTriggered: thisPage.state = "default"
    }
    contents: TextField {
        id: searchField
        anchors {
            left: parent ? parent.left : undefined
            right: parent ? parent.right : undefined
            rightMargin: units.gu(2)
        }
        color: styleMusic.common.black
        hasClearButton: true
        inputMethodHints: Qt.ImhNoPredictiveText
        placeholderText: i18n.tr("Search music")

        onVisibleChanged: {
            if (visible) {
                forceActiveFocus()
            }
        }

        // Use the page onVisible as the text field goes visible=false when switching states
        // This is used when popping from the pageStack and returning back to a page with search
        Connections {
            target: thisPage

            onStateChanged: {  // ensure the search is reset (eg pressing Esc)
                if (state === "default") {
                    searchField.text = ""
                }

                // FIXME: Workaround for pad.lv/1514143 (keyboard show/hide on view moving)
                // by locking the header and forcing a topMargin of page to the header height
                //headerState.head.locked = state === headerState.name;
                //thisPage.anchors.topMargin = state === headerState.name ? units.gu(6.125) : 0  // FIXME: 6.125 is header.height
            }

            onVisibleChanged: {
                // clear when the page becomes visible not invisible
                // if invisible is used the delegates can be destroyed which
                // have created the pushed component
                if (visible) {
                    thisPage.state = "default"
                }
            }
        }
    }

    property Page thisPage
    property alias query: searchField.text
}
