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
import "Delegates"
import "Flickables"
import "ListItemActions"

Item {
    property alias listview: queueList

    MultiSelectListView {
        id: queueList
        anchors {
            fill: parent
        }
        footer: Item {
            height: mainView.height - (styleMusic.common.expandHeight + queueList.currentHeight) + units.gu(8)
        }
        model: player.trackQueue.model
        objectName: "nowPlayingqueueList"

        delegate: MusicListItem {
            id: queueListItem
            color: player.currentIndex === index ? "#2c2c34" : styleMusic.mainView.backgroundColor
            imageSource: makeCoverSource(model.art, model.author, model.album)
            column: Column {
                Label {
                    id: trackTitle
                    color: player.currentIndex === index ? UbuntuColors.blue : styleMusic.common.music
                    fontSize: "small"
                    objectName: "titleLabel"
                    text: model.title
                }

                Label {
                    id: trackArtist
                    color: styleMusic.common.subtitle
                    fontSize: "x-small"
                    objectName: "artistLabel"
                    text: model.author
                }
            }
            leadingActions: ListItemActions {
                actions: [
                    Remove {
                        onTriggered: {
                            mainView.currentlyWorking = true
                            delayRemoveTrackFromQueue.start()
                        }
                    }
                ]
            }
            multiselectable: false
            objectName: "nowPlayingListItem" + index
            reorderable: true
            trailingActions: ListItemActions {
                actions: [
                    ShowInfo {
                    },
                    AddToPlaylist {
                    }
                ]
                delegate: ActionDelegate {
                }
            }

            Timer {
                id: delayRemoveTrackFromQueue
                interval: 100
                onTriggered: {
                    removeTrackFromQueue(model)
                    mainView.currentlyWorking = false
                }
            }

            onItemClicked: {
                mainView.currentlyWorking = true
                delayIndexQueueClicked.start()
            }

            Timer {
                id: delayIndexQueueClicked
                interval: 100
                onTriggered: {
                    indexQueueClicked(index) // toggle track state
                    mainView.currentlyWorking = false
                }
            }
        }

        onReorder: {
            delayReorderTrackInQueue.argFrom = from
            delayReorderTrackInQueue.argTo = to
            mainView.currentlyWorking = true
            delayReorderTrackInQueue.start()
        }

        Timer {
            id: delayReorderTrackInQueue
            interval: 100
            property int argFrom: 0
            property int argTo: 0
            onTriggered: {
                reorderTrackInQueue(argFrom, argTo)
                mainView.currentlyWorking = false
            }
        }

    }

    Scrollbar {
        flickableItem: queueList
        align: Qt.AlignTrailing
    }
}
