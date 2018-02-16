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
    name: "default"

    property alias newServiceEnabled: newServiceAction.visible
    property alias searchEnabled: searchAction.visible
    property PageHeader thisHeader: PageHeader {
        id: headerState
        flickable: thisPage.pageFlickable
        leadingActionBar {
            actions: {
                if (mainPageStack.currentPage === tabs) {
                    tabs.tabActions
                } else if (mainPageStack.depth > 1) {
                    backActionComponent
                } else {
                    null
                }
            }
            objectName: "tabsLeadingActionBar"
        }
        title: thisPage.pageTitle
        trailingActionBar {
            actions: [
                Action {
                    id: newServiceAction
                    objectName: "newServiceButton"
                    iconName: "add"
                    onTriggered: {
                        customdebug("New service.")
                        mainPageStack.push(Qt.resolvedUrl("AddService.qml"))
                    }
                },
                Action {
                    id: searchAction
                    iconName: "search"
                    onTriggered: {
                        thisPage.state = "search";
                        thisPage.header.contents.forceActiveFocus();
                    }
                }
            ]
            objectName: "serviceTrailingActionBar"
        }
        visible: thisPage.state === "default"

        Action {
            id: backActionComponent
            iconName: "back"
            objectName: "backAction"
            onTriggered: mainPageStack.goBack()
        }

        StyleHints {
            backgroundColor: mainView.headerColor
            dividerColor: Qt.darker(mainView.headerColor, 1.1)
        }
    }
    property Item thisPage

    PropertyChanges {
        target: thisPage
        header: thisHeader
    }
}
