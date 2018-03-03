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
import Ubuntu.Components.Popups 1.3
import Ubuntu.Thumbnailer 0.1
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"
import "../components/ListItemActions"


MusicPage {
    id: favoritesPage
    objectName: "favoritesPage"

    property bool isListView: false

    pageTitle: i18n.tr("Favorites")
    pageFlickable: favoriteGrid.visible ? favoriteGrid : favoriteList
    searchable: true
    searchResultsCount: favoritesModelFilter.count
    state: "default"
    states: [
        SearchableHeadState {
            thisPage: favoritesPage
            searchEnabled: favoritesModelFilter.count > 0
            thisHeader {
                extension: DefaultSections { }
            }
        },
        MultiSelectHeadState {
            listview: favoriteList
            thisPage: favoritesPage
            addToQueue: false
            addToPlaylist: false
            removable: true
            thisHeader {
                extension: DefaultSections { }
            }

            onRemoved: {
                customdebug("remove selected indices from favorites");
            }
        },
        SearchHeadState {
            id: searchHeader
            thisPage: favoritesPage
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    width: mainPageStack.width

    SortFilterModel {
        id: favoritesModelFilter
        model: AllFavoritesModel
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
        id: favoriteList
        anchors {
            bottomMargin: units.gu(2)
            fill: parent
            topMargin: units.gu(2)
        }
        model: favoritesModelFilter

        onStateChanged: {
            if (state === "multiselectable") {
                favoritesPage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                favoritesPage.state = "default"
            }
        }

        delegate: MusicListItem {
            id: favoriteItem
            objectName: "favoritesPageListItem" + index
            column: Column {
                Label {
                    id: favoriteTitle
                    color: styleMusic.common.music
                    fontSize: "small"
                    objectName: "itemtitle"
                    text: model.title
                }

                Label {
                    id: favoriteDescription
                    color: styleMusic.common.subtitle
                    fontSize: "x-small"
                    text: model.description.length > 0 ? model.description :
                          model.type === 1 ? i18n.tr("Album") :
                          model.type === 2 ? i18n.tr("Artist") :
                          model.type === 3 ? i18n.tr("Genre") :
                          model.type === 4 ? i18n.tr("Playlist") :
                          model.type === 5 ? i18n.tr("Song") :
                          "";
                }
            }
            leadingActions: ListItemActions {
                actions: [
                    Remove {
                        onTriggered: {
                            delayRemoveFavorite.start()
                        }
                    }
                ]
            }

            height: units.gu(7)

            noCover: model.type === 5 && !model.canQueue ? Qt.resolvedUrl("../graphics/radio.png") :
                     model.type === 2 ? Qt.resolvedUrl("../graphics/none.png") :
                     Qt.resolvedUrl("../graphics/no_cover.png")

            imageSource: model.type === 1 ? makeCoverSource(model.art, model.artist, model.title) :
                         model.type === 2 ? makeCoverSource(undefined, model.artist, undefined) :
                         model.type === 5 && model.canQueue ? makeCoverSource(model.art, model.author, model.album) :
                         model.type === 5 && model.art === "" ? Qt.resolvedUrl("../graphics/radio.png") :
                         makeCoverSource(model.art, undefined, undefined)

            onImageError: model.art = "" // reset invalid url from model

            multiselectable: false

            trailingActions: ListItemActions {
                actions: [
                    Action {
                        iconName: "add"
                        objectName: "addToQueueAction"
                        text: i18n.tr("Add to queue")
                        visible: model.canQueue
                        onTriggered: addQueue({id: model.objectId, payload: model.object})
                    },
                    Action {
                        iconName: "add-to-playlist"
                        objectName: "addToPlaylistAction"
                        text: i18n.tr("Add to playlist")
                        visible: model.canQueue
                        onTriggered: {
                            mainPageStack.push(Qt.resolvedUrl("AddToPlaylist.qml"),
                                               {"chosenElements": [{id: model.objectId, payload: model.object}]})
                        }
                    }
                ]
                delegate: ActionDelegate {
                }
            }

            Timer {
                id: delayRemoveFavorite
                interval: 100
                onTriggered: {
                    if (!player.removeFavorite(model.id))
                        popInfo.open(i18n.tr("Action can't be performed"));
                }
            }

            onItemClicked: clickItem(model)
        }

        opacity: isListView ? 1.0 : 0.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

    MusicGridView {
        id: favoriteGrid
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: favoritesModelFilter

        onStateChanged: {
            if (state === "multiselectable") {
                favoritesPage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                favoritesPage.state = "default"
            }
        }

        delegate: Card {
            id: favoriteCard
            primaryText: model.title
            secondaryText: model.description.length > 0 ? model.description :
                           model.type === 1 ? i18n.tr("Album") :
                           model.type === 2 ? i18n.tr("Artist") :
                           model.type === 3 ? i18n.tr("Genre") :
                           model.type === 4 ? i18n.tr("Playlist") :
                           model.type === 5 ? i18n.tr("Song") :
                           ""

            noCover: model.type === 5 && !model.canQueue ? Qt.resolvedUrl("../graphics/radio.png") :
                     model.type === 2 ? Qt.resolvedUrl("../graphics/none.png") :
                     Qt.resolvedUrl("../graphics/no_cover.png")

            coverSources: model.type === 1 ? [{art: makeCoverSource(model.art, model.artist, model.title)}, {art: makeCoverSource(undefined, model.artist, model.title)}]
                        : model.type === 2 ? [{art: makeCoverSource(undefined, model.artist, undefined)}]
                        : model.type === 5 && model.canQueue ? [{art: makeCoverSource(model.art, model.author, model.album)}, {art: makeCoverSource(undefined, model.author, model.album)}]
                        : model.type === 5 && model.art === "" ? [{art: Qt.resolvedUrl("../graphics/radio.png")}]
                        : [{art: makeCoverSource(model.art, undefined, undefined)}]

            onImageError: model.art = "" // reset invalid url from model
            onClicked: clickItem(model)
            onPressAndHold: {
                favoritesPage.isListView = true
            }
        }

        opacity: isListView ? 0.0 : 1.0
        visible: opacity > 0.0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
    }

    Timer {
        id: delayFavoritePlayAll
        interval: 100
        property QtObject model
        onTriggered: {
            // clear queue when playing bundle
            if (model.type !== 5 && model.canQueue) {
                if (player.removeAllTracksFromQueue())
                    player.playFavorite(model);
            } else {
               player.playFavorite(model);
            }
        }
    }

    function clickItem(model) {
        if (!model.isService) {
            if (model.type === 1) {
                mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                   {
                                       "containerItem": {id: model.objectId, payload: model.object},
                                       "songSearch": model.objectId,
                                       "album": model.album,
                                       "artist": model.artist,
                                       "covers": [{art: makeCoverSource(model.art, model.artist, model.album)}],
                                       "isAlbum": true,
                                       "genre": "",
                                       "pageTitle": i18n.tr("Album"),
                                       "line1": model.artist !== undefined ? model.artist : "",
                                       "line2": model.album !== undefined ? model.album : i18n.tr("Unknown Album")
                                   })
            }
            else if (model.type === 2) {
                mainPageStack.push(Qt.resolvedUrl("ArtistView.qml"),
                                   {
                                       "containerItem": {id: model.objectId, payload: model.object},
                                       "artistSearch": model.objectId,
                                       "artist": model.artist,
                                       "covers": [{art: makeCoverSource(undefined, model.artist, undefined)}],
                                       "pageTitle": i18n.tr("Artist")
                                   })
            }
            else if (model.type === 3) {
                mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                   {
                                       "containerItem": {id: model.objectId, payload: model.object},
                                       "songSearch": model.objectId + "//",
                                       "covers": [],
                                       "album": "",
                                       "genre": model.title,
                                       "pageTitle": i18n.tr("Genre"),
                                       "line1": "",
                                       "line2": model.title
                                   })
            }
            else if (model.type === 4) {
                mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                   {
                                       "containerItem": {id: model.objectId, payload: model.object},
                                       "songSearch": model.objectId,
                                       "album": "",
                                       "covers": [{art: makeCoverSource(model.art, model.artist, model.album)}],
                                       "isPlaylist": true,
                                       "genre": "",
                                       "page": playlistsPage,
                                       "pageTitle": i18n.tr("Playlist"),
                                       "line1": "",
                                       "line2": model.title
                                   })
            }
            else if (model.type === 5) {
                player.playFavorite(model) // play it
            }
        } else {
            delayFavoritePlayAll.model = model
            delayFavoritePlayAll.start()
        }
    }
}

