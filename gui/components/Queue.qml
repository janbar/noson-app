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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQml.Models 2.3
import "Delegates"
import "Flickables"
import "ListItemActions"

Item {
    id: queue
    property alias listview: queueList
    property alias header: queueList.header
    property alias headerItem: queueList.headerItem
    clip: true

    Component {
        id: dragDelegate

        DragMusicListItem {
            id: listItem
            listview: queueList
            color: palette.base
            highlightedColor: palette.highlight
            highlighted: (player.currentIndex === index)

            onSwipe: {
                listview.focusIndex = index > 0 ? index - 1 : 0;
                removeTrackFromQueue(model);
                color = "red";
            }

            onReorder: {
                listview.reorder(from, to)
            }

            onClick: dialogSongInfo.open(model, false) // don't show actions

            imageSource: makeCoverSource(model.art, model.author, model.album)
            description: qsTr("Song")

            onImageError: model.art = "" // reset invalid url from model
            onActionPressed: indexQueueClicked(model.index)
            actionVisible: true
            actionIconSource: (player.isPlaying && player.currentIndex === index ? "qrc:/images/media-playback-pause.svg" : "qrc:/images/media-preview-start.svg")
            menuVisible: true

            menuItems: [
                AddToFavorites {
                    enabled: !model.isService
                    visible: enabled
                    description: listItem.description
                    art: model.art
                },
                //@FIXME add to playlist service item doesn't work
                AddToPlaylist {
                    enabled: !model.isService
                    visible: enabled
                },
                Remove {
                    onTriggered: {
                        listview.focusIndex = index > 0 ? index - 1 : 0;
                        removeTrackFromQueue(model);
                        color = "red";
                    }
                }
            ]

            column: Column {
                Label {
                    id: trackTitle
                    color: palette.text
                    font.pointSize: units.fs("small")
                    font.bold: true
                    text: model.title
                }

                Label {
                    id: trackArtist
                    color: styleMusic.common.subtitle
                    font.pointSize: units.fs("x-small")
                    text: model.author
                }

            }

        }

    }

    MusicListView {
        id: queueList
        objectName: "queueList"
        anchors.fill: parent

        footer: Item {
            height: mainView.height - (styleMusic.common.expandHeight + queueList.currentHeight) + units.gu(8)
        }

        model: DelegateModel {
            id: visualModel
            model: player.trackQueue.model
            delegate: dragDelegate
        }

        property int focusIndex: 0

        Connections {
            target: player.trackQueue.model
            onLoaded: {
                if (queueList.focusIndex > 0) {
                    queueList.positionViewAtIndex(queueList.focusIndex, ListView.Center);
                    queueList.focusIndex = 0;
                } else {
                    queueList.positionViewAtIndex(player.currentIndex > 0 ? player.currentIndex - 1 : 0, ListView.Beginning);
                }
            }
        }

        signal reorder(int from, int to)

        onReorder: {
            customdebug("Reorder queue item " + from + " to " + to);
            queueList.focusIndex = to;
            reorderTrackInQueue(from, to);
        }

    }

}
