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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import NosonApp 1.0
import NosonMediaScanner 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"


MusicPage {
    id: genresPage
    objectName: "genresPage"
    pageFlickable: genreGridView
    searchable: true

    GenreList {
        id: genres
        Component.onCompleted: init()
    }

    onSearchClicked: filter.visible = true

    header: MusicFilter {
        id: filter
        visible: false
    }

    MusicGridView {
        id: genreGridView
        itemWidth: units.gu(12)
        heightOffset: units.gu(7)

        model: SortFilterModel {
            model: genres
            sort.property: "genre"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
            filter.property: "normalized"
            filter.pattern: new RegExp(normalizedInput(filter.displayText), "i")
            filterCaseSensitivity: Qt.CaseInsensitive
        }

        property int delayed: 0

        delegate: Card {
            id: genreCard
            height: genreGridView.cellHeight
            width: genreGridView.cellWidth
            coversGridVisible: true
            coverSources: []
            coverFlow: 4
            objectName: "genresPageGridItem" + index
            primaryText: (model.genre !== "<Undefined>" ? model.genre : tr_undefined)
            secondaryTextVisible: false

            onClicked: {
                stackView.push("qrc:/controls2/ThisDevice/SongsView.qml",
                                   {
                                       "coverFlow": 4,
                                       "covers": coverSources,
                                       "album": undefined,
                                       "genre": model.genre,
                                       "pageTitle": pageTitle,
                                       "line1": "",
                                       "line2": (model.genre !== "<Undefined>" ? model.genre : tr_undefined)
                                   })
            }
            onPressAndHold: {
            }
        }
    }
}
