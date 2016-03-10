/*
 * Copyright (C) 2016
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
import Ubuntu.Thumbnailer 0.1
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"
import "../components/ListItemActions"


MusicPage {
    id: radiosPage
    objectName: "radiosPage"
    title: i18n.tr("My radios")
    searchable: true
    searchResultsCount: radiosModelFilter.count
    state: "default"
    states: [
        SearchableHeadState {
            thisPage: radiosPage
            searchEnabled: radiosModelFilter.count > 0
        },
        MultiSelectHeadState {
            listview: radiolist
            thisPage: radiosPage
            addToQueue: false
            addToPlaylist: false
            removable: true

            onRemoved: {
                customdebug("remove selected indices from radios");
            }

        },
        SearchHeadState {
            id: searchHeader
            thisPage: radiosPage
        }
    ]

    // Hack for autopilot otherwise Albums appears as MusicPage
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

    MultiSelectListView {
        id: radiolist
        anchors {
            bottomMargin: units.gu(2)
            fill: parent
            topMargin: units.gu(2)
        }
        objectName: "radiostab-listview"
        model: SortFilterModel {
            id: radiosModelFilter
            model: AllRadiosModel
            sort.property: "title"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        onStateChanged: {
            if (state === "multiselectable") {
                radiosPage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                radiosPage.state = "default"
            }
        }

        delegate: MusicListItem {
            id: radio
            objectName: "radiosPageListItem" + index
            column: Column {
                Label {
                    id: radioTitle
                    color: styleMusic.common.music
                    fontSize: "small"
                    objectName: "tracktitle"
                    text: model.title
                }

                Label {
                    id: radioGenre
                    color: styleMusic.common.subtitle
                    fontSize: "x-small"
                    text: /*model.genre !== undefined && model.genre !== "" ? model.genre : */ model.streamId
                }
            }
            height: units.gu(7)
            noCover: Qt.resolvedUrl("../graphics/streaming.svg")
            imageSource: {"art": model.icon !== "" ? model.icon
                                                   : model.streamId !== undefined && model.streamId !== ""
                                                   ? "http://cdn-radiotime-logos.tunein.com/" + model.streamId + "q.png"
                                                   : undefined
            }
            multiselectable: true
            trailingActions: ListItemActions {
                actions: [
                    AddToFavorites {
                        description: i18n.tr("Radio")
                    }
                ]
                delegate: ActionDelegate {
                }
            }

            onItemClicked: {
                mainView.currentlyWorking = true
                delayRadioClicked.start()
            }

            Timer {
                id: delayRadioClicked
                interval: 100
                onTriggered: {
                    radioClicked(model) // play radio
                    mainView.currentlyWorking = false
                }
            }
        }
    }
}

