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
import NosonApp 1.0
import "Delegates"
import "Flickables"
import "ListItemActions"

Item {
    id: queue
    property QueueModel queueModel: null
    property alias listview: queueList
    property alias header: queueList.header
    property alias headerItem: queueList.headerItem
    property alias backgroundColor: bg.color
    clip: true

    Rectangle {
        id: bg
        anchors.fill: parent
        color: "transparent"
    }

    function positionAt(index) {
        // index is in view ?
        if (index >= queueModel.firstIndex &&
                index < queueModel.firstIndex + queueModel.count) {
            // move view at desired position - 1
            var i = index - queueModel.firstIndex - 1;
            queueList.positionViewAtIndex(i < 0 ? 0 : i, ListView.Beginning);
        } else {
            // set desired focus after reloading
            queueList.focusId = index;
            queueList.focusMode = ListView.Beginning;
            // reload starting at position - 1
            queueModel.fetchAt(index - 1);
        }
    }

    Component {
        id: dragDelegate

        DragMusicListItem {
            id: listItem
            listview: queueList
            color: bg.color
            highlightedColor: styleMusic.view.highlightedColor
            highlighted: (player.currentIndex === model.trackIndex)

            onSwipe: {
                listview.focusId = model.trackIndex > 0 ? model.trackIndex - 1 : 0;
                listview.focusMode = ListView.Center;
                removeTrackFromQueue(model);
                color = "red";
            }

            onReorder: {
                listview.reorder(from, to)
            }

            onClick: dialogSongInfo.open(model, [{art: imageSource}],
                                         "qrc:/controls2/Library.qml",
                                         {
                                             "rootPath": "A:ARTIST/" + model.author,
                                             "rootTitle": model.author,
                                             "rootType": LibraryModel.NodePerson,
                                             "isListView": false,
                                             "displayType": LibraryModel.DisplayGrid
                                         })

            imageSources: makeCoverSource(model.art, model.author, model.album)
            description: qsTr("Song")

            onImageError: model.art = "" // reset invalid url from model
            onActionPressed: indexQueueClicked(model.trackIndex)
            actionVisible: true
            actionIconSource: (player.isPlaying && player.currentIndex === model.trackIndex ? "qrc:/images/media-playback-pause.svg" : "qrc:/images/media-preview-start.svg")
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
                        listview.focusId = model.trackIndex > 0 ? model.trackIndex - 1 : 0;
                        listview.focusMode = ListView.Center;
                        removeTrackFromQueue(model);
                        color = "red";
                    }
                }
            ]

            coverSize: units.gu(5)

            column: Column {
                Label {
                    id: trackIndex
                    color: styleMusic.view.primaryColor
                    font.pointSize: units.fs("small")
                    text: "# " + (model.trackIndex + 1)
                }

                Label {
                    id: trackTitle
                    color: styleMusic.view.primaryColor
                    font.pointSize: units.fs("medium")
                    text: model.title
                }

                Label {
                    id: trackArtist
                    color: styleMusic.view.secondaryColor
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
            height: mainView.height - (styleMusic.view.expandHeight + queueList.currentHeight) + units.gu(8)
        }

        model: DelegateModel {
            id: visualModel
            model: queueModel
            delegate: dragDelegate
        }

        property int focusId: 0
        property int focusMode: ListView.Center

        Connections {
            target: queueModel
            function onDataUpdated() {
                // save current focus
                queueList.focusId = queueModel.firstIndex + queueList.indexAt(queueList.contentX, queueList.contentY);
                queueList.focusMode = ListView.Beginning;
            }
            function onViewUpdated() {
                // move to the saved fosus
                if (queueList.focusId > 0) {
                    queueList.positionViewAtIndex(queueList.focusId - queueModel.firstIndex, queueList.focusMode);
                    // clear saved focus
                    queueList.focusId = 0;
                }
            }
        }

        signal reorder(int from, int to)

        onReorder: {
            customdebug("Reorder queue item " + from + " to " + to);
            queueList.focusId = to + queueModel.firstIndex;
            queueList.focusMode = ListView.Center;
            reorderTrackInQueue(queueModel.firstIndex + from, queueModel.firstIndex + to);
        }

        onAtYEndChanged: {
            if (queueList.atYEnd && queueModel.totalCount > (queueModel.firstIndex + queueModel.count)) {
                if (queueList.focusId === 0 && queueModel.fetchBack()) {
                    queueList.focusId = queueModel.firstIndex + queueModel.count;
                    queueList.focusMode = ListView.End;
                }
            }
        }

        onAtYBeginningChanged: {
            if (queueList.atYBeginning && queueModel.firstIndex > 0) {
                if (queueList.focusId === 0 && queueModel.fetchFront()) {
                    queueList.focusId = queueModel.firstIndex - 1;
                    queueList.focusMode = ListView.Beginning;
                }
            }
        }

        Component.onCompleted: {
            // FIX move up triggering
            if (queueList.atYBeginning && queueModel.firstIndex > 0) {
                queueList.positionViewAtIndex(1, ListView.Beginning);
            }
        }
    }
}
