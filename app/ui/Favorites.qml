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
    title: i18n.tr("Favorites")
    searchable: true
    searchResultsCount: favoritesModelFilter.count
    state: "default"
    states: [
        SearchableHeadState {
            thisPage: favoritesPage
            searchEnabled: favoritesModelFilter.count > 0
        },
        MultiSelectHeadState {
            listview: favoritelist
            thisPage: favoritesPage
            addToQueue: false
            addToPlaylist: false
            removable: true

            onRemoved: {
                customdebug("remove selected indices from favorites");
            }

        },
        SearchHeadState {
            id: searchHeader
            thisPage: favoritesPage
        }
    ]

    // Hack for autopilot otherwise Albums appears as MusicPage
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

    MultiSelectListView {
        id: favoritelist
        anchors {
            bottomMargin: units.gu(2)
            fill: parent
            topMargin: units.gu(2)
        }
        objectName: "favoritestab-listview"
        model: SortFilterModel {
            id: favoritesModelFilter
            model: AllFavoritesModel
            sort.property: "title"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        onStateChanged: {
            if (state === "multiselectable") {
                favoritesPage.state = "selection"
            } else {
                searchHeader.query = ""  // force query back to default
                favoritesPage.state = "default"
            }
        }

        delegate: MusicListItem {
            id: favorite
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
                    text: model.description
                }
            }
            leadingActions: ListItemActions {
                actions: [
                    Remove {
                        onTriggered: {
                            mainView.currentlyWorking = true
                            delayRemoveFavorite.start()
                        }
                    }
                ]
            }

            height: units.gu(7)

            noCover: model.type === 5 && !model.canQueue ? Qt.resolvedUrl("../graphics/streaming.svg") : Qt.resolvedUrl("../graphics/no_cover.png")

            imageSource: model.type === 1 ? {art: model.art, artist: model.artist, album: model.title} :
                         model.type === 2 ? {artist: model.artist} :
                         model.type === 5 && model.art !== "" ? {art: model.art, artist: model.author, album: model.album} :
                         {art: model.art}

            multiselectable: false

            trailingActions: ListItemActions {
                actions: [
                    Action {
                        iconName: "add"
                        objectName: "addToQueueAction"
                        text: i18n.tr("Add to queue")
                        enabled: model.canQueue
                        onTriggered: addQueue({id: model.objectId, payload: model.object})
                    },
                    Action {
                        iconName: "add-to-playlist"
                        objectName: "addToPlaylistAction"
                        text: i18n.tr("Add to playlist")
                        enabled: model.canQueue
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
                    mainView.currentlyWorking = false
                }
            }

            onItemClicked: {
                if (model.type === 1) {
                    mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                       {
                                           "containerItem": {id: model.objectId, payload: model.object},
                                           "songSearch": model.objectId,
                                           "album": model.album,
                                           "artist": model.artist,
                                           "covers": [{art: model.art, artist: model.artist, album: model.album}],
                                           "isAlbum": true,
                                           "genre": undefined,
                                           "title": i18n.tr("Album"),
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
                                           "covers": [{artist: model.artist}],
                                           "title": i18n.tr("Artist")
                                       })
                }
                else if (model.type === 3) {
                    mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                       {
                                           "containerItem": {id: model.objectId, payload: model.object},
                                           "songSearch": model.objectId + "//",
                                           "covers": [],
                                           "album": undefined,
                                           "genre": model.title,
                                           "title": i18n.tr("Genre"),
                                           "line1": "",
                                           "line2": model.title
                                       })
                }
                else if (model.type === 4) {
                    mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                       {
                                           "containerItem": {id: model.objectId, payload: model.object},
                                           "songSearch": model.objectId,
                                           "album": undefined,
                                           "covers": [{art: model.art, artist: model.artist, album: model.album}],
                                           "isPlaylist": true,
                                           "genre": undefined,
                                           "page": playlistsPage,
                                           "title": i18n.tr("Playlist"),
                                           "line1": "",
                                           "line2": model.title
                                       })
                }
                else if (model.type === 5) {
                    mainView.currentlyWorking = true
                    delayfavoriteClicked.start()
                }
            }

            Timer {
                id: delayfavoriteClicked
                interval: 100
                onTriggered: {
                    player.playFavorite(model) // play favorite
                    mainView.currentlyWorking = false
                }
            }

        }
    }
}

