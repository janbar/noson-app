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
import Ubuntu.Components.Popups 1.3
import Ubuntu.Thumbnailer 0.1
import NosonApp 1.0
import "../components"
import "../components/Delegates"
import "../components/Flickables"
import "../components/ViewButton"
import "../components/HeadState"

MusicPage {
    id: artistViewPage
    objectName: "artistViewPage"
    visible: false
    pageFlickable: artistAlbumView

    property var containerItem: null

    property string artistSearch: ""
    property string artist: ""
    property var covers: []
    property bool loaded: false  // used to detect difference between first and further loads

    width: mainPageStack.width

    property bool isFavorite: false

    state: "artist"
    states: [
        ArtistAlbumsHeadState {
            thisPage: artistViewPage
            thisHeader {
                extension: DefaultSections { }
            }
        }
    ]

    MusicGridView {
        id: artistAlbumView
        itemWidth: units.gu(15)
        heightOffset: units.gu(7)
        header: Item {
            height: blurredHeader.height
            width: parent.width

            // Put BlurredHeader in contain so we can remove the margins
            BlurredHeader {
                id: blurredHeader
                anchors {
                    left: parent.left
                    margins: -units.gu(1)
                    right: parent.right
                    top: parent.top
                }
                bottomColumn: Column {
                    Label {
                        id: artistLabel
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        color: styleMusic.common.music
                        elide: Text.ElideRight
                        fontSize: "x-large"
                        maximumLineCount: 1
                        objectName: "artistLabel"
                        text: artist != "" ? artist : i18n.tr("Unknown Artist")
                        wrapMode: Text.NoWrap
                    }

                    Item {
                        height: units.gu(1)
                        width: parent.width
                    }

                    Row {
                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        Label {
                            id: albumCount
                            color: styleMusic.common.subtitle
                            elide: Text.ElideRight
                            fontSize: "small"
                            maximumLineCount: 1
                            text: i18n.tr("%1 album", "%1 albums", albumsModel.count).arg(albumsModel.count)
                        }

                        Label {
                            id: separator
                            color: styleMusic.common.subtitle
                            elide: Text.ElideRight
                            fontSize: "small"
                            maximumLineCount: 1
                            text: " , "
                            visible: songArtistModel.count > 0
                        }

                        Label {
                            id: songCount
                            color: styleMusic.common.subtitle
                            elide: Text.ElideRight
                            fontSize: "small"
                            maximumLineCount: 1
                            text: i18n.tr("%1 song", "%1 songs", songArtistModel.totalCount).arg(songArtistModel.totalCount)
                            visible: songArtistModel.count > 0
                        }
                    }
                }
                coverSources: artistViewPage.covers
                height: blurredHeader.width > units.gu(60) ? units.gu(33.5) : ((blurredHeader.width - units.gu(5)) / 2) + units.gu(12)
                rightColumn: Column {
                    spacing: units.gu(2)
                    ShuffleButton {
                        model: songArtistModel
                        width: blurredHeader.width > units.gu(60) ? units.gu(23.5) : (blurredHeader.width - units.gu(13)) / 2
                    }
                    QueueAllButton {
                        containerItem: artistViewPage.containerItem
                        width: blurredHeader.width > units.gu(60) ? units.gu(23.5) : (blurredHeader.width - units.gu(13)) / 2
                    }
                    PlayAllButton {
                        containerItem: artistViewPage.containerItem
                        width: blurredHeader.width > units.gu(60) ? units.gu(23.5) : (blurredHeader.width - units.gu(13)) / 2
                    }
                }
            }            
        }
        model: AlbumsModel {
            id: albumsModel
            Component.onCompleted: init(Sonos, artistSearch, true)
        }
        delegate: Card {
            id: albumCard
            coverSources: [{art: makeCoverSource(model.art, model.artist, model.title)}]
            objectName: "albumsPageGridItem" + index
            primaryText: model.title !== "" ? model.title : i18n.tr("Unknown Album")
            secondaryTextVisible: false

            onClicked: {
                mainPageStack.push(Qt.resolvedUrl("SongsView.qml"),
                                   {
                                       "containerItem": makeContainerItem(model),
                                       "songSearch": model.id,
                                       "album": model.title,
                                       "artist": model.artist,
                                       "covers": [{art: albumCard.imageSource}],
                                       "isAlbum": true,
                                       "genre": "",
                                       "pageTitle": i18n.tr("Album"),
                                       "line1": model.artist !== undefined && model.artist !== "" ? model.artist : i18n.tr("Unknown Artist"),
                                       "line2": model.title !== undefined && model.title !== "" ? model.title : i18n.tr("Unknown Album")
                                   })
            }
        }
    }

    TracksModel {
        id: songArtistModel
    }

    Timer {
        id: delayInitModel
        interval: 100
        onTriggered: {
            isFavorite = (AllFavoritesModel.findFavorite(containerItem.id) !== "")
            songArtistModel.init(Sonos, artistSearch + "/", true)
            mainView.currentlyWorking = false
            loaded = true
        }
    }

    Component.onCompleted: {
        mainView.currentlyWorking = true
        delayInitModel.start()
    }
}

