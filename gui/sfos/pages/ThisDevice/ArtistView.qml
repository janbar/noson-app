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
import QtQuick 2.2
import Sailfish.Silica 1.0

import NosonApp 1.0
import NosonMediaScanner 1.0
import "../../components"
import "../../components/Delegates"
import "../../components/Flickables"

MusicPage {
    id: artistViewPage
    objectName: "artistViewPage"
    visible: false
    pageFlickable: artistAlbumView

    property string artist: ""
    property var covers: []

    property bool isFavorite: false

    AlbumList {
        id: albums
        artist: artistViewPage.artist
        Component.onCompleted: init()
    }

    function makeFileCoverSource(hasArt, filePath, artist, album) {
        var art = "";
        if (hasArt)
            art = player.makeFilePictureLocalURL(filePath);
        return makeCoverSource(art, artist, album);
    }

    MusicGridView {
        id: artistAlbumView
        itemWidth: units.gu(15)
        heightOffset: units.gu(7)
        clip: true
        header: MusicHeader {
            id: blurredHeader
            height: contentHeight
            noCover: "qrc:/images/none.png"
            coverSources: artistViewPage.covers
            titleColumn: Item {}
            rightColumn: Item {
                height: units.gu(6)

                Row {
                    id: r1
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    Label {
                        id: artistLabel
                        color: styleMusic.view.primaryColor
                        elide: Text.ElideRight
                        font.pixelSize: units.fx("x-large")
                        maximumLineCount: 1
                        text: (artist !== "<Undefined>" ? artist : tr_undefined)
                        wrapMode: Text.NoWrap
                    }
                }

                Row {
                    id: r2
                    anchors {
                        topMargin: units.gu(0.5)
                        top: r1.bottom
                        left: parent.left
                        right: parent.right
                    }

                    Label {
                        id: albumCount
                        color: styleMusic.view.secondaryColor
                        elide: Text.ElideRight
                        font.pixelSize: units.fx("small")
                        maximumLineCount: 1
                        text: qsTr("%n album(s)", "", albums.count)
                    }

                    Label {
                        id: separator
                        color: styleMusic.view.secondaryColor
                        elide: Text.ElideRight
                        font.pixelSize: units.fx("small")
                        maximumLineCount: 1
                        text: " , "
                        visible: songArtistModel.count > 0
                    }

                    Label {
                        id: songCount
                        color: styleMusic.view.secondaryColor
                        elide: Text.ElideRight
                        font.pixelSize: units.fx("small")
                        maximumLineCount: 1
                        text: qsTr("%n song(s)", "", songArtistModel.count)
                        visible: songArtistModel.count > 0
                    }
                }
            }
        }

        model: SortFilterModel {
            model: albums
            sort.property: "album"
            sort.order: Qt.AscendingOrder
            sortCaseSensitivity: Qt.CaseInsensitive
        }

        delegate: Card {
            id: albumCard
            coverSources: makeFileCoverSource(model.hasArt, model.filePath, model.artist, model.album)
            primaryText: (model.album !== "<Undefined>" ? model.album : tr_undefined)
            secondaryTextVisible: false

            onImageError: model.art = "" // reset invalid url from model
            onClicked: {
                pageStack.push("qrc:/sfos/pages/ThisDevice/SongsView.qml",
                                   {
                                       "album": model.album,
                                       "artist": model.artist,
                                       "covers": albumCard.imageSource != "" ? [{art: albumCard.imageSource}] : coverSources,
                                       "isAlbum": true,
                                       "genre": "",
                                       "pageTitle": pageTitle,
                                       "line1": (model.artist !== "<Undefined>" ? model.artist : tr_undefined),
                                       "line2": (model.album !== "<Undefined>" ? model.album : tr_undefined)
                                   })
            }
        }
    }

    // Query total count of artist's songs
    TrackList {
        id: songArtistModel
        artist: artistViewPage.artist
        Component.onCompleted: init()
    }
}
