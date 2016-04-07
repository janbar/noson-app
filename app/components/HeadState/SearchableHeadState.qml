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
import Ubuntu.Components.Popups 1.3

PageHeadState {
    name: "default"
    head: thisPage.head
    actions: [
        Action {
        id: searchAction
        iconName: "search"
        onTriggered: thisPage.state = "search"
        },
        Action {
            //iconName: "settings"
            iconSource: Qt.resolvedUrl("../../graphics/cogs.svg")
            objectName: "queueActions"
            // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
            text: i18n.tr("Manage queue")
            visible: mainView.wideAspect && player.trackQueue.model.count > 0
            onTriggered: {
                currentDialog = PopupUtils.open(Qt.resolvedUrl("../Dialog/DialogManageQueue.qml"), mainView)
            }
        },
        Action {
            //iconName: "clock"
            iconSource: Qt.resolvedUrl("../../graphics/timer.svg")
            objectName: "timerActions"
            // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
            text: i18n.tr("Standby timer")
            visible: mainView.wideAspect
            onTriggered: {
                currentDialog = PopupUtils.open(Qt.resolvedUrl("../Dialog/DialogSleepTimer.qml"), mainView)
            }
        },
        Action {
            //iconName: "import"
            iconSource: Qt.resolvedUrl("../../graphics/input.svg")
            objectName: "inputActions"
            // TRANSLATORS: this action appears in the overflow drawer with limited space (around 18 characters)
            text: i18n.tr("Select source")
            visible: mainView.wideAspect
            onTriggered: {
                currentDialog = PopupUtils.open(Qt.resolvedUrl("../Dialog/DialogSelectSource.qml"), mainView)
            }
        }
    ]

    property alias searchEnabled: searchAction.enabled
    property Page thisPage
}
