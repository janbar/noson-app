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

State {
    name: "search"

    property PageHeader thisHeader: PageHeader {
        id: headerState
        contents: TextField {
            id: searchField
            anchors {
                left: parent ? parent.left : undefined
                right: parent ? parent.right : undefined
                verticalCenter: parent ? parent.verticalCenter : undefined
            }
            color: theme.palette.selected.baseText
            focus: true
            hasClearButton: true
            inputMethodHints: Qt.ImhNoPredictiveText
            placeholderText: i18n.tr("Search music")

            // Use the page onVisible as the text field goes visible=false when switching states
            // This is used when popping from the pageStack and returning back to a page with search
            Connections {
                target: thisPage

                onStateChanged: {  // ensure the search is reset (eg pressing Esc)
                    if (state === "default") {
                        searchField.text = ""
                    }
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
        flickable: thisPage.pageFlickable
        leadingActionBar {
            actions: [
                Action {
                    id: leaveSearchAction
                    text: "back"
                    iconName: "back"
                    onTriggered: thisPage.state = "default"
                }
            ]
        }
        trailingActionBar {
            actions: [
                Action {
                    visible: thisPage.isListView !== undefined
                    iconName: thisPage.isListView ? "view-grid-symbolic" : "view-list-symbolic"
                    // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
                    text: i18n.tr("Show list")
                    onTriggered: {
                        thisPage.isListView = !thisPage.isListView
                    }
                }
            ]
        }
        visible: thisPage.state === "search"

        onVisibleChanged: {
            if (visible) {
                forceActiveFocus()
            }
        }

        StyleHints {
            backgroundColor: mainView.headerColor
            dividerColor: Qt.darker(mainView.headerColor, 1.1)
        }
    }
    property Item thisPage
    property alias query: searchField.text

    PropertyChanges {
        target: thisPage
        header: thisHeader
    }
}
