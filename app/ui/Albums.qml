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
    id: albumsPage
    objectName: "albumsPage"
    title: i18n.tr("Albums")
    searchable: true
    searchResultsCount: albumsModelFilter.count
    state: "default"
    states: [
        SearchableHeadState {
            thisPage: albumsPage
            searchEnabled: albumsModelFilter.count > 0
        },
        SearchHeadState {
            id: searchHeader
            thisPage: albumsPage
        }
    ]

    // FIXME: workaround for pad.lv/1531016 (gridview juddery)
    anchors {
        fill: undefined
    }
    height: mainView.height
    width: mainView.width

    // Hack for autopilot otherwise Albums appears as MusicPage
    // due to bug 1341671 it is required that there is a property so that
    // qml doesn't optimise using the parent type
    property bool bug1341671workaround: true

    MusicGridView {
        id: albumGridView
        itemWidth: units.gu(15)
        heightOffset: units.gu(9.5)

        model: SortFilterModel {
            id: albumsModelFilter
            model: AllAlbumsModel
            sort.property: "title"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(searchHeader.query), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }
        delegate: Card {
            id: albumCard
            coverSources: [{art: model.art}]
            objectName: "albumsPageGridItem" + index
            primaryText: model.title !== undefined && model.title !== "" ? model.title : i18n.tr("Unknown Album")
            secondaryText: model.artist !== undefined && model.artist !== "" ? model.artist : i18n.tr("Unknown Artist")

            onClicked: {
                mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "songSearch": model.id,
                                       "album": model.title,
                                       "artist": model.artist,
                                       "covers": [{art: model.art}],
                                       "isAlbum": true,
                                       "genre": undefined,
                                       "title": i18n.tr("Album"),
                                       "line1": model.artist !== undefined && model.artist !== "" ? model.artist : i18n.tr("Unknown Artist"),
                                       "line2": model.title !== undefined && model.title !== "" ? model.title : i18n.tr("Unknown Album")
                                   })
            }
        }
    }
}
