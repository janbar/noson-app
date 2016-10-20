/*
 * Copyright (C) 2016
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
import Ubuntu.Components.Popups 1.3

State {
    id: state
    name: stateName

    // Need to be able to change state name and State.name is not notifyable
    property string stateName: "default"
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
                    iconName: isListView ? "view-fullscreen" : "media-playlist"
                    // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
                    text: i18n.tr("Show queue")
                    visible: !mainView.wideAspect && queueLoader.status === Loader.Ready
                    onTriggered: {
                        isListView = !isListView
                    }
                },
                Action {
                    //iconName: "settings"
                    iconSource: Qt.resolvedUrl("../../graphics/cogs.svg")
                    objectName: "queueActions"
                    // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
                    text: i18n.tr("Manage queue")
                    visible: player.trackQueue.model.count > 0
                    onTriggered: {
                        thisPage.currentDialog = PopupUtils.open(Qt.resolvedUrl("../Dialog/DialogManageQueue.qml"), thisPage)
                    }
                },
                Action {
                    //iconName: "clock"
                    iconSource: Qt.resolvedUrl("../../graphics/timer.svg")
                    objectName: "timerActions"
                    // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
                    text: i18n.tr("Standby timer")
                    visible: true
                    onTriggered: {
                        thisPage.currentDialog = PopupUtils.open(Qt.resolvedUrl("../Dialog/DialogSleepTimer.qml"), thisPage)
                    }
                },
                Action {
                    //iconName: "import"
                    iconSource: Qt.resolvedUrl("../../graphics/input.svg")
                    objectName: "inputActions"
                    // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
                    text: i18n.tr("Select source")
                    visible: true
                    onTriggered: {
                        thisPage.currentDialog = PopupUtils.open(Qt.resolvedUrl("../Dialog/DialogSelectSource.qml"), thisPage)
                    }
                }
            ]
        }
        visible: thisPage.state === state.stateName

        Action {
            id: backActionComponent
            iconName: "back"
            objectName: "backAction"
            onTriggered: mainPageStack.pop()
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
