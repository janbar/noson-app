/*
 * Copyright (C) 2013, 2014, 2015, 2016
 *      Jean-Luc Barriere <jlbarriere68@gmail.com>
 *      Andrew Hayzen <ahayzen@gmail.com>
 *      Daniel Holm <d.holmen@gmail.com>
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
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"


MusicPage {
    id: playlistsPage
    objectName: "playlistsPage"
    // TRANSLATORS: this is the name of the playlists page shown in the tab header.
    // Remember to keep the translation short to fit the screen width
    pageTitle: i18n.tr("Playlists")
    pageFlickable: playlistslist
    searchable: true
    searchResultsCount: playlistModelFilter.count
    state: "default"
    states: [
        PlaylistsHeadState {
            newPlaylistEnabled: true
            searchEnabled: AllPlaylistsModel.count > 0
            thisPage: playlistsPage
            thisHeader {
                extension: DefaultSections { }
            }
        },
        SearchHeadState {
            id: searchHeader
            thisPage: playlistsPage
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    width: mainPageStack.width

    property bool changed: false
    property bool childrenChanged: false

    MusicGridView {
        id: playlistslist
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)
        model: SortFilterModel {
            // Sorting disabled as it is incorrect on first run (due to workers?)
            // and SQL sorts the data correctly
            id: playlistModelFilter
            model: AllPlaylistsModel
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }
        objectName: "playlistsGridView"
        delegate: Card {
            id: playlistCard
            coverSources: makeCoverSources()

            function makeCoverSources() {
                var cs = [];
                var arts = model.arts;
                for (var i = 0; i < arts.length; ++i) cs.push({art: arts[i]});
                return cs;
            }

            coverFlow: 4
            objectName: "playlistCardItem" + index
            primaryText: model.title
            secondaryTextVisible: false
            isFavorite: (AllFavoritesModel.findFavorite(model.id).length > 0)

            // check favorite on data updated
            Connections {
                target: AllFavoritesModel
                onDataUpdated: {
                    isFavorite = (AllFavoritesModel.findFavorite(model.id).length > 0)
                }
            }

            onClicked: {
                mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "songSearch": model.id,
                                       "album": undefined,
                                       "covers": coverSources,
                                       "isPlaylist": true,
                                       "genre": undefined,
                                       "page": playlistsPage,
                                       "pageTitle": i18n.tr("Playlist"),
                                       "line1": "",
                                       "line2": model.title,
                                   })
            }
            onPressAndHold: {
                if (isFavorite && removeFromFavorites(model))
                    isFavorite = false
                else if (!isFavorite && addItemToFavorites(model, i18n.tr("Playlist"), imageSource))
                    isFavorite = true
            }
        }
    }

    // Overlay to show when no playlists are on the device
    Loader {
        anchors {
            fill: parent
            topMargin: -units.gu(6.125)  // FIXME: 6.125 is the header.height
        }
        active: AllPlaylistsModel.count === 0 && !infoLoadedIndex
        asynchronous: true
        source: "../components/PlaylistsEmptyState.qml"
        visible: active
    }
}
