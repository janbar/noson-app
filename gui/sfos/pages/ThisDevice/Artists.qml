/*
 * Copyright (C) 2019
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

import QtQuick 2.2
import Sailfish.Silica 1.0

import QtQuick.Layouts 1.1
import NosonApp 1.0
import NosonMediaScanner 1.0
import "../../components"
import "../../components/Delegates"
import "../../components/Flickables"


MusicPage {
    id: artistsPage
    objectName: "artistsPage"
    pageFlickable: artistGridView
    searchable: true

    ArtistList {
        id: artists
        Component.onCompleted: init()
    }

    onSearchClicked: filter.visible = true

    header: MusicFilter {
        id: filter
        visible: false
    }

    MusicGridView {
        id: artistGridView
        itemWidth: units.gu(12)
        heightOffset: units.gu(7)

        model: SortFilterModel {
            model: artists
            sort.property: "artist"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: Card {
            id: artistCard
            coverSources: makeCoverSource(undefined, model.artist, undefined)
            noCover: "qrc:/images/none.png"
            objectName: "artistsPageGridItem" + index
            primaryText: (model.artist !== "<Undefined>" ? model.artist : tr_undefined)
            secondaryTextVisible: false

            onClicked: {
                stackView.push("qrc:/ui/ThisDevice/ArtistView.qml",
                                   {
                                       "artist": model.artist,
                                       "covers": [{art: artistCard.imageSource}],
                                       "pageTitle": pageTitle
                                   })
            }
        }
    }
}
