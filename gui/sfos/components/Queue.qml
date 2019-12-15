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

import QtQuick 2.2
import Sailfish.Silica 1.0
import QtQml.Models 2.2
import "Delegates"
import "Flickables"
import "ListItemActions"

Item {
    id: queue
    property alias listview: queueList
    property alias header: queueList.header
    property alias headerItem: queueList.headerItem
    property alias backgroundColor: bg.color
    property alias backgroundOpacity: bg.opacity
    clip: true

    Rectangle {
        id: bg
        anchors.fill: parent
        color: "transparent"
        opacity: 1.0
    }

    Component {
        id: dragDelegate

        DragMusicListItem {
            id: listItem
            listview: queueList
            color: bg.color
            highlightedColor: styleMusic.view.highlightedColor
            highlighted: (player.currentIndex === index)

            onSwipe: {
                listview.focusIndex = index > 0 ? index - 1 : 0;
                removeTrackFromQueue(model);
                color = "red";
            }

            onReorder: {
                listview.reorder(from, to)
            }

            onClick: dialogSongInfo.open(model, [{art: imageSource}], false, true) // don't show action 'play'

            imageSources: makeCoverSource(model.art, model.author, model.album)
            description: qsTr("Song")

            onImageError: model.art = "" // reset invalid url from model
            onActionPressed: indexQueueClicked(model.index)
            actionVisible: true
            actionIconSource: (player.isPlaying && player.currentIndex === index ? "qrc:/images/media-playback-pause.svg" : "qrc:/images/media-preview-start.svg")
            menuVisible: true
/*!TODO
            Component.onCompleted: {
                console.debug("Populating menu")
                
                var component = Qt.createComponent("AddToFavorites.qml");
                var newMenuItem = component.createObject(menuItems, {"enabled" : !model.isService, "visible": enabled, "description": listItem.description, "art": model.art })

                component = Qt.createComponent("AddToPlaylist.qml");
                newMenuItem = component.createObject(menuItems, {"enabled" : !model.isService, "visible": enabled })
                
                
            }
*/

//                Remove {
 //                   onTriggered: {
 //                       listview.focusIndex = index > 0 ? index - 1 : 0;
 ///                       removeTrackFromQueue(model);
 //                       color = "red";
 //                   }
 //               }
            

            column: Column {
                Label {
                    id: trackTitle
                    color: styleMusic.view.primaryColor
                    font.pointSize: units.fs("medium")
                    font.bold: true
                    text: model.title
                }

                Label {
                    id: trackArtist
                    color: styleMusic.view.secondaryColor
                    font.pointSize: units.fs("small")
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
            height: mainView.height - (styleMusic.view.expandHeight + queueList.currentHeight) + units.gu(8)
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
