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
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"
import "../components/ListItemActions"


MusicPage {
    id: radiosPage
    objectName: "radiosPage"

    property bool isListView: false

    // used to detect view has updated properties since first load.
    // - isFavorite
    property bool taintedView: false

    pageTitle: i18n.tr("My radios")
    pageFlickable: radioGrid.visible ? radioGrid : radioList
    searchable: true
    searchResultsCount: radiosModelFilter.count
    state: "default"
    states: [
        SearchableHeadState {
            thisPage: radiosPage
            searchEnabled: radiosModelFilter.count > 0
            thisHeader {
                extension: DefaultSections { }
            }
        },
        MultiSelectHeadState {
            listview: radioList
            thisPage: radiosPage
            addToQueue: false
            addToPlaylist: false
            removable: true
            thisHeader {
                extension: DefaultSections { }
            }

            onRemoved: {
                customdebug("remove selected indices from radios");
            }
        },
        SearchHeadState {
            id: searchHeader
            thisPage: radiosPage
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    width: mainPageStack.width

    SortFilterModel {
        id: radiosModelFilter
        model: AllRadiosModel
        sort.property: "title"
        sort.order: Qt.AscendingOrder
        sortCaseSensitivity: Qt.CaseInsensitive
        filter.property: "normalized"
        filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
        filterCaseSensitivity: Qt.CaseInsensitive
    }

    Timer {
        id: delayLoadModel
        interval: 100
        onTriggered: {
            AllRadiosModel.load();
            radiosPage.taintedView = false; // reset
            mainView.currentlyWorking = false;
        }
    }

    // Hack for autopilot otherwise Albums appears as MusicPage
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

    MultiSelectListView {
        id: radioList
        anchors {
            fill: parent
            topMargin: units.gu(2)
            bottomMargin: units.gu(2)
        }
        model: radiosModelFilter

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

            noCover: Qt.resolvedUrl("../graphics/radio.png")

            imageSource: model.icon !== "" ? model.icon :
                         model.streamId !== undefined && model.streamId !== "" ? "http://cdn-radiotime-logos.tunein.com/" + model.streamId + "q.png" :
                         ""

            multiselectable: true

            trailingActions: ListItemActions {
                actions: [
                    Action {
                        property bool isFavorite: false
                        property string description: i18n.tr("Radio")
                        property string art: imageSource

                        iconName: isFavorite ? "starred" : "scope-manager"
                        objectName: "ActionFavorite"
                        text: i18n.tr("Favorite")

                        Component.onCompleted: {
                            isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                        }

                        onTriggered: {
                            if (isFavorite && removeFromFavorites(model.payload))
                                isFavorite = false;
                            else if (!isFavorite && addItemToFavorites(model, description, art))
                                isFavorite = true;
                            radiosPage.taintedView = true;
                        }
                    }
                ]
                delegate: ActionDelegate {
                }
            }

            onItemClicked: {
                mainView.currentlyWorking = true
                delayRadioClicked.model = model
                delayRadioClicked.start()
            }

        }

        opacity: isListView ? 1.0 : 0.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

    MusicGridView {
        id: radioGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: radiosModelFilter

        onStateChanged: {
            if (state === "multiselectable") {
                radiosPage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                radiosPage.state = "default"
            }
        }

        delegate: Card {
            id: radioCard
            primaryText: model.title
            secondaryText: /*model.genre !== undefined && model.genre !== "" ? model.genre : */ model.streamId
            isFavorite: (AllFavoritesModel.findFavorite(model.payload).length > 0)

            // check favorite on data updated
            Connections {
                target: AllFavoritesModel
                onDataUpdated: {
                    isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            noCover: Qt.resolvedUrl("../graphics/radio.png")
            coverSources: [{art: model.icon !== "" ? model.icon :
                         model.streamId !== undefined && model.streamId !== "" ? "http://cdn-radiotime-logos.tunein.com/" + model.streamId + "q.png" :
                         ""}]

            onClicked: {
                mainView.currentlyWorking = true
                delayRadioClicked.model = model
                delayRadioClicked.start()
            }
            onPressAndHold: {
                if (isFavorite && removeFromFavorites(model.payload))
                    isFavorite = false;
                else if (!isFavorite && addItemToFavorites(model, i18n.tr("Radio"), imageSource))
                    isFavorite = true;
                radiosPage.taintedView = true;
            }
        }

        opacity: isListView ? 0.0 : 1.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

    Timer {
        id: delayRadioClicked
        interval: 100
        property QtObject model
        onTriggered: {
            radioClicked(model) // play radio
            mainView.currentlyWorking = false
        }
    }
}

