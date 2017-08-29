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
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"
import "../components/ListItemActions"


MusicPage {
    id: servicesPage
    objectName: "servicesPage"

    property bool isListView: false

    pageTitle: i18n.tr("My services")
    pageFlickable: serviceGrid.visible ? serviceGrid : serviceList
    searchable: true
    searchResultsCount: servicesModelFilter.count
    state: "default"
    states: [
        SearchableHeadState {
            thisPage: servicesPage
            searchEnabled: servicesModelFilter.count > 0
            thisHeader {
                extension: DefaultSections { }
            }
        },
        MultiSelectHeadState {
            listview: serviceList
            thisPage: servicesPage
            addToQueue: false
            addToPlaylist: false
            removable: false
            thisHeader {
                extension: DefaultSections { }
            }
        },
        SearchHeadState {
            id: searchHeader
            thisPage: servicesPage
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    width: mainPageStack.width

    SortFilterModel {
        id: servicesModelFilter
        model: AllServicesModel
        sort.property: "title"
        sort.order: Qt.AscendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
        filter.property: "normalized"
        filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    // Hack for autopilot otherwise Albums appears as MusicPage
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

    MultiSelectListView {
        id: serviceList
        anchors {
            fill: parent
            topMargin: units.gu(2)
            bottomMargin: units.gu(2)
        }
        model: servicesModelFilter

        onStateChanged: {
            if (state === "multiselectable") {
                servicesPage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                servicesPage.state = "default"
            }
        }

        delegate: MusicListItem {
            id: service
            objectName: "servicesPageListItem" + index
            column: Column {
                Label {
                    id: serviceTitle
                    color: styleMusic.common.music
                    fontSize: "small"
                    objectName: "servicetitle"
                    text: model.title
                }

                Label {
                    id: serviceNickName
                    color: styleMusic.common.subtitle
                    fontSize: "x-small"
                    text: model.nickName
                }
            }

            height: units.gu(7)

            noCover: Qt.resolvedUrl("../graphics/radio.png")

            imageSource: model.id === "SA_RINCON65031_" ? Qt.resolvedUrl("../graphics/tunein.png") : model.icon

            multiselectable: false

            onItemClicked: {
                mainPageStack.push(Qt.resolvedUrl("Service.qml"),
                                   {
                                       "serviceItem": model,
                                       "pageTitle": model.title,
                                   })
            }
        }

        opacity: isListView ? 1.0 : 0.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

    MusicGridView {
        id: serviceGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: servicesModelFilter

        onStateChanged: {
            if (state === "multiselectable") {
                servicesPage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                servicesPage.state = "default"
            }
        }

        delegate: Card {
            id: serviceCard
            primaryText: model.title
            secondaryText: model.nickName
            isFavorite: false

            noCover: Qt.resolvedUrl("../graphics/radio.png")
            coverSources: [{art: model.id === "SA_RINCON65031_" ? Qt.resolvedUrl("../graphics/tunein.png") : model.icon}]

            onClicked: {
                mainPageStack.push(Qt.resolvedUrl("Service.qml"),
                                   {
                                       "serviceItem": model,
                                       "pageTitle": model.title
                                   })
            }
        }

        opacity: isListView ? 0.0 : 1.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

}

