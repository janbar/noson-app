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
import Ubuntu.Thumbnailer 0.1
import NosonApp 1.0
//import "../logic/meta-database.js" as Library
//import "../logic/playlists.js" as Playlists
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/HeadState"


MusicPage {
    id: artistsPage
    objectName: "artistsPage"
    title: i18n.tr("Artists")
    searchable: true
    searchResultsCount: artistsModelFilter.count
    state: "default"
    states: [
        SearchableHeadState {
            thisPage: artistsPage
            searchEnabled: artistsModelFilter.count > 0
        },
        SearchHeadState {
            id: searchHeader
            thisPage: artistsPage
        }
    ]

    // FIXME: workaround for pad.lv/1531016 (gridview juddery)
    anchors {
        fill: undefined
    }
    height: mainView.height
    width: mainView.width

    // Hack for autopilot otherwise Artists appears as MusicPage
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

    MusicGridView {
        id: artistGridView
        itemWidth: units.gu(12)
        heightOffset: units.gu(7)
        model: SortFilterModel {
            id: artistsModelFilter
            model: AllArtistsModel
            sort.property: "artist"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }
        delegate: Card {
            id: artistCard
            coverSources: [{artist: model.artist}]
            objectName: "artistsPageGridItem" + index
            primaryText: model.artist !== undefined && model.artist !== "" ? model.artist : i18n.tr("Unknown Artist")
            secondaryTextVisible: false

            onClicked: {
                mainPageStack.push(Qt.resolvedUrl("ArtistView.qml"),
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "artistSearch": model.id,
                                       "artist": model.artist,
                                       "covers": [{art: artistCard.imageSource}],
                                       "title": i18n.tr("Artist")
                                   })
            }
        }
    }
}

