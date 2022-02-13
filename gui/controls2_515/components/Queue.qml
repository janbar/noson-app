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
import "../../toolbox.js" as ToolBox

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
            queueList.focusView(index - 1, ListView.Beginning);
        } else {
            // reload starting at position - 1
            queueList.saveViewFocus(index, ListView.Beginning);
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
                var focusId = model.trackIndex - 1;
                queueList.saveViewFocus(focusId, ListView.Center);
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
                        var focusId = model.trackIndex - 1;
                        queueList.saveViewFocus(focusId, ListView.Center);
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

        Connections {
            target: queueModel
            function onDataUpdated() {
                var focusId = queueModel.firstIndex + queueList.indexAt(queueList.contentX, queueList.contentY);
                queueList.saveViewFocus(focusId, ListView.Beginning);
            }
            //function onViewUpdated() {
            //}
        }

        property bool fetchEnabled: false // property to enable/disable fetch on move

        function focusView(focusId, focusMode) {
            var index = (focusId < queueModel.firstIndex ? 0 : focusId - queueModel.firstIndex);
            positionViewAtIndex(index, focusMode);
            // finally enable fetch on move
            fetchEnabled = true;
        }

        function saveViewFocus(focusId, focusMode) {
            // disable fetch on move
            fetchEnabled = false;
            ToolBox.connectOnce(queueModel.onViewUpdated, function(){
                queueList.focusView(focusId, focusMode);
            });
        }

        signal reorder(int from, int to)

        onReorder: {
            customdebug("Reorder queue item " + from + " to " + to);
            var focusId = to + queueModel.firstIndex;
            queueList.saveViewFocus(focusId, ListView.Center);
            reorderTrackInQueue(queueModel.firstIndex + from, queueModel.firstIndex + to);
        }

        onAtYEndChanged: {
            if (fetchEnabled && queueList.atYEnd &&
                    queueModel.totalCount > (queueModel.firstIndex + queueModel.count)) {
                if (queueModel.fetchBack()) {
                    var focusId = queueModel.firstIndex + queueModel.count;
                    queueList.saveViewFocus(focusId, ListView.End);
                }
            }
        }

        onAtYBeginningChanged: {
            if (fetchEnabled && queueList.atYBeginning &&
                    queueModel.firstIndex > 0) {
                if (queueModel.fetchFront()) {
                    var focusId = queueModel.firstIndex - 1;
                    queueList.saveViewFocus(focusId, ListView.Beginning);
                }
            }
        }

        Component.onCompleted: {
            // FIX move up triggering
            if (queueList.atYBeginning && queueModel.firstIndex > 0) {
                queueList.positionViewAtIndex(1, ListView.Beginning);
            }
            queueList.fetchEnabled = true;
        }
    }
}
