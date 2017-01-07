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
import Ubuntu.Thumbnailer 0.1
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"
import "../components/ListItemActions"


MusicPage {
    id: servicePage
    objectName: "servicePage"

    property var serviceItem: null
    property bool loaded: false  // used to detect difference between first and further loads
    property bool isRoot: mediaModel.isRoot
    property int displayType: 0 /*Grid*/
    property bool isListView: false

    // used to detect view has updated properties since first load.
    // - isFavorite
    property bool taintedView: false

    pageTitle: serviceItem.title
    pageFlickable: mediaGrid.visible ? mediaGrid : mediaList
    searchable: true
    searchResultsCount: mediaModel.count
    state: "default"
    states: [
        ServiceHeadState {
            thisPage: servicePage
            searchEnabled: false /*mediaModel.count > 0*/
            thisHeader {
                extension: DefaultSections { }
            }
        },
        MultiSelectHeadState {
            listview: mediaList
            thisPage: servicePage
            addToQueue: false
            addToPlaylist: false
            removable: false
            thisHeader {
                extension: DefaultSections { }
            }
        },
        SearchHeadState {
            id: searchHeader
            thisPage: servicePage
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    width: mainPageStack.width

    BlurredBackground {
            id: blurredBackground
            height: parent.height
            art: serviceItem.id == "SA_RINCON65031_" ? Qt.resolvedUrl("../graphics/tunein.png") : serviceItem.icon
    }

    MediaModel {
      id: mediaModel
    }

    Timer {
        id: delayInitModel
        interval: 100
        onTriggered: {
            mediaModel.init(Sonos, serviceItem.payload, true)
            mainView.currentlyWorking = false
            servicePage.loaded = true;
        }
    }

    Timer {
        id: delayLoadModel
        interval: 100
        onTriggered: {
            mediaModel.load();
            servicePage.taintedView = false; // reset
            mainView.currentlyWorking = false;
        }
    }

    Timer {
        id: delayLoadMore
        interval: 100
        onTriggered: {
            mediaModel.loadMore()
            mainView.currentlyWorking = false
        }
    }

    Connections {
        target: mediaModel
        onDataUpdated: {
            mainView.currentlyWorking = true
            delayLoadModel.start()
        }
    }

    Connections {
        target: mediaModel
        onIsRootChanged: {
          if (mediaModel.isRoot)
            pageTitle = serviceItem.title;
          else
            pageTitle = serviceItem.title + " : " + mediaModel.path();
        }
    }

    onDisplayTypeChanged: {
        isListView = (displayType === 0 /*Grid*/ || displayType === 3 /*Editorial*/) ? false : true
    }

    // Hack for autopilot otherwise Albums appears as MusicPage
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

    MultiSelectListView {
        id: mediaList
        anchors {
            bottomMargin: units.gu(2)
            fill: parent
            topMargin: units.gu(2)
        }
        model: mediaModel

        onStateChanged: {
            if (state === "multiselectable") {
                servicePage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                servicePage.state = "default"
            }
        }

        delegate: MusicListItem {
            id: mediaItem
            color: "transparent"
            property string description: model.description.length > 0 ? model.description
                                       : model.type === 1 ? i18n.tr("Album")
                                       : model.type === 2 ? i18n.tr("Artist")
                                       : model.type === 3 ? i18n.tr("Genre")
                                       : model.type === 4 ? i18n.tr("Playlist")
                                       : model.type === 5 && model.canQueue ? i18n.tr("Song")
                                       : model.type === 5 ? i18n.tr("Radio")
                                       : ""
            column: Column {
                Label {
                    id: mediaTitle
                    color: styleMusic.common.music
                    fontSize: "medium"
                    objectName: "itemtitle"
                    text: model.title
                }

                Label {
                    id: mediaDescription
                    color: styleMusic.common.subtitle
                    fontSize: "small"
                    text: description
                }
            }
            leadingActions: ListItemActions {
                actions: [
                ]
            }

            height: units.gu(7)

            noCover: model.type === 2 ? Qt.resolvedUrl("../graphics/none.png")
                   : model.canPlay && !model.canQueue ? Qt.resolvedUrl("../graphics/radio.png")
                   : Qt.resolvedUrl("../graphics/no_cover.png")

            imageSource: model.art !== "" ? model.art
                       : model.type === 2 ? Qt.resolvedUrl("../graphics/none.png")
                       : model.canPlay && !model.canQueue ? Qt.resolvedUrl("../graphics/radio.png")
                       : Qt.resolvedUrl("../graphics/no_cover.png")

            multiselectable: false

            trailingActions: ListItemActions {
                actions: [
                    Action {
                        iconName: "add"
                        objectName: "addToQueueAction"
                        text: i18n.tr("Add to queue")
                        visible: model.canQueue
                        onTriggered: addQueue({id: model.Id, payload: model.payload})
                    },
                    Action {
                        iconName: "add-to-playlist"
                        objectName: "addToPlaylistAction"
                        text: i18n.tr("Add to playlist")
                        visible: model.canQueue
                        onTriggered: {
                            mainPageStack.push(Qt.resolvedUrl("AddToPlaylist.qml"),
                                               {"chosenElements": [{id: model.Id, payload: model.payload}]})
                        }
                    },
                    Action {
                        property bool isFavorite: false

                        iconName: isFavorite ? "starred" : "scope-manager"
                        objectName: "ActionFavorite"
                        text: i18n.tr("Favorite")
                        visible: model.canPlay

                        Component.onCompleted: {
                            isFavorite = model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false
                        }

                        onTriggered: {
                            if (isFavorite && removeFromFavorites(model.payload))
                                isFavorite = false;
                            else if (!isFavorite && addItemToFavorites(model, mediaItem.description, mediaItem.imageSource))
                                isFavorite = true;
                            servicePage.taintedView = true;
                        }
                    }
                ]
                delegate: ActionDelegate {
                }
            }

            onItemClicked: clickItem(model)
        }

        visible: isListView ? true : false

        onAtYEndChanged: {
            if (mediaList.atYEnd && mediaModel.totalCount > mediaModel.count) {
                mainView.currentlyWorking = true
                delayLoadMore.start()
            }
        }
    }

    MusicGridView {
        id: mediaGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: mediaModel

        onStateChanged: {
            if (state === "multiselectable") {
                servicePage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                servicePage.state = "default"
            }
        }

        delegate: Card {
            id: favoriteCard
            primaryText: model.title
            secondaryText: model.description.length > 0 ? model.description
                         : model.type === 1 ? i18n.tr("Album")
                         : model.type === 2 ? i18n.tr("Artist")
                         : model.type === 3 ? i18n.tr("Genre")
                         : model.type === 4 ? i18n.tr("Playlist")
                         : model.type === 5 && model.canQueue ? i18n.tr("Song")
                         : model.type === 5 ? i18n.tr("Radio")
                         : ""
            isFavorite: model.canPlay ? (AllFavoritesModel.findFavorite(model.payload).length > 0) : false

            noCover: model.type === 2 ? Qt.resolvedUrl("../graphics/none.png")
                   : model.canPlay && !model.canQueue ? Qt.resolvedUrl("../graphics/radio.png")
                   : Qt.resolvedUrl("../graphics/no_cover.png")

            coverSources: model.art !== "" ? [{art: model.art}]
                        : model.type === 2 ? [{art: Qt.resolvedUrl("../graphics/none.png")}]
                        : model.canPlay && !model.canQueue ? [{art: Qt.resolvedUrl("../graphics/radio.png")}]
                        : [{art: Qt.resolvedUrl("../graphics/no_cover.png")}]

            onClicked: clickItem(model)
            onPressAndHold: {
                if (model.canPlay) {
                    if (isFavorite && removeFromFavorites(model.payload))
                        isFavorite = false;
                    else if (!isFavorite && addItemToFavorites(model, secondaryText, imageSource))
                        isFavorite = true;
                    servicePage.taintedView = true;
                } else {
                    servicePage.isListView = true
                }
            }
        }

        visible: isListView ? false : true

        onAtYEndChanged: {
            if (mediaGrid.atYEnd && mediaModel.totalCount > mediaModel.count) {
                mainView.currentlyWorking = true
                delayLoadMore.start()
            }
        }
    }

    Component.onCompleted: {
        mainView.currentlyWorking = true
        delayInitModel.start()
    }

    Timer {
        id: delayGoUp
        interval: 100
        onTriggered: {
            // change view depending of previous display type
            servicePage.displayType = mediaModel.previousDisplayType();
            mediaModel.loadParent();
            mainView.currentlyWorking = false
        }
    }

    function goUp() {
        mainView.currentlyWorking = true
        delayGoUp.start()
    }

    Timer {
        id: delayMediaClicked
        interval: 100
        property QtObject model
        onTriggered: {
            if (model.canPlay) {
                if (model.canQueue)
                  trackClicked(model);
                else
                  radioClicked(model);
            } else if (model.isContainer) {
                var old = servicePage.displayType;
                servicePage.displayType = model.displayType;
                mediaModel.loadChild(model.id, model.title, old);
            }
            mainView.currentlyWorking = false
        }
    }

    function clickItem(model) {
        mainView.currentlyWorking = true
        delayMediaClicked.model = model
        delayMediaClicked.start()
    }
}

