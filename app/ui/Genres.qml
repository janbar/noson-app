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
    id: genresPage
    objectName: "genresPage"
    pageTitle: i18n.tr("Genres")
    pageFlickable: genreGridView
    searchable: true
    searchResultsCount: genresModelFilter.count
    state: "default"
    states: [
        SearchableHeadState {
            thisPage: genresPage
            searchEnabled: genresModelFilter.count > 0
            thisHeader {
                extension: DefaultSections { }
            }
        },
        SearchHeadState {
            id: searchHeader
            thisPage: genresPage
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    width: mainPageStack.width

    // Hack for autopilot otherwise Albums appears as MusicPage
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

    MusicGridView {
        id: genreGridView
        itemWidth: units.gu(12)
        heightOffset: units.gu(7)
        model: SortFilterModel {
            id: genresModelFilter
            model: AllGenresModel
            filter.property: "normalized"
            filter.pattern: searchHeader.query === "" ? /\S+/ : new RegExp(normalizedInput(searchHeader.query), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
            sort.property: "genre"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
        }

        AlbumsModel {
            id: childModel
        }
        property var artworks: []

        function findCoverSources(modelItem) {
            var covers = [];
            var root = modelItem.id + "/";
            // register and load directory content for root
            childModel.init(Sonos, root, true);
            var count = childModel.count;
            var index = 0;
            while (index < count && index < 4) {
                var childItem = childModel.get(index);
                covers.push({art: makeCoverSource(childItem.art, childItem.artist, undefined)});
                ++index;
            }
            artworks[modelItem.genre] = covers;
            // unregister directory content
            childModel.init(null, root);
            return covers;
        }

        delegate: Card {
            id: genreCard
            coversGridVisible: true
            coverSources: []
            coverFlow: 4
            objectName: "genresPageGridItem" + index
            primaryText: model.genre || i18n.tr("<Undefined>")
            secondaryTextVisible: false
            isFavorite: (AllFavoritesModel.findFavorite(model.payload).length > 0)

            // check favorite on data updated
            Connections {
                target: AllFavoritesModel
                onDataUpdated: {
                    isFavorite = (AllFavoritesModel.findFavorite(model.payload).length > 0)
                }
            }

            Component.onCompleted: {
                // read from artworks cache
                var covers = genreGridView.artworks[model.genre];
                if (covers !== undefined)
                    coverSources = covers
                else
                    delayArtwork.start()
            }

            Timer {
                id: delayArtwork
                interval: 1000
                onTriggered: {
                    coverSources = genreGridView.findCoverSources(model);
                }
            }

            onClicked: {
                mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "songSearch": model.id + "//",
                                       "covers": coverSources,
                                       "album": "",
                                       "genre": model.genre,
                                       "pageTitle": i18n.tr("Genre"),
                                       "line1": "",
                                       "line2": model.genre
                                   })
            }
            onPressAndHold: {
                if (isFavorite && removeFromFavorites(model.payload))
                    isFavorite = false
                else if (!isFavorite && addItemToFavorites(model, i18n.tr("Genre"), imageSource))
                    isFavorite = true
            }
        }
    }
}
