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

State {
    name: "default"

    property alias searchEnabled: searchAction.enabled
    property PageHeader thisHeader: PageHeader {
        id: headerState
        flickable: thisPage.pageFlickable
        leadingActionBar {
            actions: {
                if (thisPage.isRoot) {
                    backActionComponent
                } else {
                    goUpActionComponent
                }
            }
            objectName: "tabsLeadingActionBar"
        }
        title: thisPage.pageTitle
        trailingActionBar {
            actions: [
                Action {
                    visible: thisPage.isListView !== undefined
                    iconName: thisPage.isListView ? "view-grid-symbolic" : "view-list-symbolic"
                    // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
                    text: i18n.tr("Show list")
                    onTriggered: {
                        thisPage.isListView = !thisPage.isListView;
                        if (thisPage.taintedView) {
                            mainView.currentlyWorking = true;
                            delayLoadModel.start();
                        }
                    }
                },
                Action {
                    id: homeAction
                    iconName: "home"
                    visible: thisPage.isRoot ? false : true
                    onTriggered: mainPageStack.goBack()
                },
                Action {
                    id: searchAction
                    iconName: "search"
                    visible: searchEnabled
                    onTriggered: {
                        thisPage.state = "search";
                        thisPage.header.contents.forceActiveFocus();
                    }
                }
            ]
        }
        visible: thisPage.state === "default"

        Action {
            id: backActionComponent
            iconName: "back"
            objectName: "backAction"
            onTriggered: mainPageStack.goBack()
        }

        Action {
            id: goUpActionComponent
            iconName: "up"
            objectName: "upAction"
            onTriggered: thisPage.goUp()
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
