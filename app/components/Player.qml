/*
 * Copyright (C) 2016
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

import QtQuick 2.4
import NosonApp 1.0

/*
 * This file should *only* manage the media playing and the relevant settings
 * It should therefore only access ZonePlayer, trackQueue and Settings
 * Anything else within the app should use Connections to listen for changes
 */


Item {
    id: player
    objectName: "controller"
    property bool connected: false
    property string currentMetaAlbum: ""
    property string currentMetaArt: ""
    property string currentMetaArtist: ""
    property string currentMetaSource: ""
    property string currentMetaTitle: ""
    property string currentMetaURITitle: ""
    property int currentIndex: -1
    property int currentCount: 0
    property int duration: 1
    readonly property bool isPlaying: player.playbackState === "PLAYING"
    readonly property var playbackState: playerLoader.status == Loader.Ready ? playerLoader.item.playbackState : ""
    property int position: 0
    property int volumeMaster: 0
    property bool repeat: false
    property bool shuffle: false
    property bool mute: false
    property int renderingControlCount: 0
    property bool sleepTimerEnabled: false

    property string queueInfo: queueOverviewString()

    signal stopped()

    onCurrentCountChanged: queueInfo = queueOverviewString()
    onCurrentIndexChanged: queueInfo = queueOverviewString()

    onIsPlayingChanged: {
        if (playingTimerLoader.status == Loader.Ready) {
              playingTimerLoader.item.running = isPlaying && duration > 1 ? true : false;
        }
    }

    onConnectedChanged: {
        if (!connected) {
            trackQueue.canLoad = false;
        }
    }

    function connect() {
        if (playerLoader.item.init(Sonos)) {
            if (trackQueue.canLoad) {
                // When switching zone, updateid cannot drive correctly the queue refreshing
                // so new force refreshing of queue
                return trackQueue.loadQueue();
            }
            return trackQueue.canLoad = true;
        }
        // dont try to refresh queue
        return trackQueue.canLoad = false;
    }

    function queueOverviewString() {
        var str = "";
        if (!connected)
            str = "- / -";
        else if (currentIndex < 0)
            str = "- / " + currentCount;
        else
            str = (currentIndex + 1) + " / " + currentCount;
        return str;
    }

    function wakeUp() {
        if (trackQueue.loadQueue()) {
            customdebug("Renew subscriptions");
            playerLoader.item.renewSubscriptions();
            return true;
        }
        return false;
    }

    function stop() {
        return playerLoader.item.stop();
    }

    function play() {
        return playerLoader.item.play();
    }

    function pause() {
        return playerLoader.item.pause();
    }

    function toggle() {
        if (isPlaying)
            return pause();
        else
            return play();
    }

    function playSong(modelItem) {
        return (setSource(modelItem) && play());
    }

    function previousSong(startPlaying) {
        return playerLoader.item.previous()
    }

    function nextSong(startPlaying, fromControls) {
        return playerLoader.item.next();
    }

    function toggleRepeat() {
        return playerLoader.item.toggleRepeat()
    }

    function toggleShuffle() {
        return playerLoader.item.toggleShuffle()
    }

    function seek(position) {
        return playerLoader.item.seekTime(Math.floor(position / 1000));
    }

    function setSource(modelItem) {
        return playerLoader.item.setSource(modelItem.payload);
    }

    function setVolumeGroup(volume) {
        return playerLoader.item.setVolumeGroup(volume);
    }

    function setVolume(uuid, volume) {
        return playerLoader.item.setVolume(uuid, volume);
    }

    function toggleMuteGroup() {
        return playerLoader.item.toggleMute();
    }

    function toggleMute(uuid) {
        return playerLoader.item.toggleMute(uuid);
    }

    function playQueue(start) {
        return playerLoader.item.playQueue(start);
    }

    function seekTrack(nr) {
        return playerLoader.item.seekTrack(nr);
    }

    function addItemToQueue(modelItem, nr) {
        return playerLoader.item.addItemToQueue(modelItem.payload, nr);
    }

    function addMultipleItemsToQueue(modelItemList) {
        var payloads = [];
        for (var i = 0; i < modelItemList.length; ++i)
            payloads.push(modelItemList[i].payload);
        return playerLoader.item.addMultipleItemsToQueue(payloads);
    }

    function removeAllTracksFromQueue() {
        return playerLoader.item.removeAllTracksFromQueue();
    }

    function removeTrackFromQueue(modelItem) {
        return playerLoader.item.removeTrackFromQueue(modelItem.id, trackQueue.model.containerUpdateID());
    }

    function reorderTrackInQueue(nrFrom, nrTo) {
        return playerLoader.item.reorderTrackInQueue(nrFrom, nrTo, trackQueue.model.containerUpdateID());
    }

    function refreshShareIndex() {
        return playerLoader.item.refreshShareIndex();
    }

    function saveQueue(title) {
        return playerLoader.item.saveQueue(title);
    }

    function createSavedQueue(title) {
        return playerLoader.item.createSavedQueue(title);
    }

    function addItemToSavedQueue(playlistId, modelItem, containerUpdateID) {
        return playerLoader.item.addItemToSavedQueue(playlistId, modelItem.payload, containerUpdateID);
    }

    function removeTracksFromSavedQueue(playlistId, selectedIndices, containerUpdateID) {
        return playerLoader.item.removeTracksFromSavedQueue(playlistId, selectedIndices, containerUpdateID);
    }

    function reorderTrackInSavedQueue(playlistId, index, newIndex, containerUpdateID) {
        return playerLoader.item.reorderTrackInSavedQueue(playlistId, index, newIndex, containerUpdateID);
    }

    function destroySavedQueue(itemId) {
        return playerLoader.item.destroySavedQueue(itemId);
    }

    function configureSleepTimer(sec) {
        return playerLoader.item.configureSleepTimer(sec);
    }

    function refreshSource() {
        // protect against undefined properties
        player.currentMetaAlbum = playerLoader.item.currentMetaAlbum || "";
        player.currentMetaArt = playerLoader.item.currentMetaArt || "";
        player.currentMetaArtist = playerLoader.item.currentMetaArtist || "";
        player.currentMetaSource = playerLoader.item.currentMetaSource || "";
        player.currentMetaTitle = playerLoader.item.currentMetaTitle || "";
        player.currentMetaURITitle = playerLoader.item.currentMetaURITitle || "";
        player.currentIndex = playerLoader.item.currentIndex;
        player.duration = 1 + (1000 * playerLoader.item.currentTrackDuration);
        // reset position
        var npos = 1000 * playerLoader.item.currentTrackPosition();
        player.position = npos > player.duration ? 0 : npos;
    }

    function refreshPlayMode() {
        player.repeat = playerLoader.item.playMode === "REPEAT_ALL" || playerLoader.item.playMode === "SHUFFLE" ? true : false;
        player.shuffle = playerLoader.item.playMode === "SHUFFLE" || playerLoader.item.playMode === "SHUFFLE_NOREPEAT" ? true : false;
    }

    function refreshRenderingGroup() {
        player.volumeMaster = playerLoader.item.volumeMaster;
        player.mute = playerLoader.item.muteMaster;
    }

    function refreshRendering() {
        renderingModelLoader.item.load(playerLoader.item);
        renderingControlCount = renderingModelLoader.item.count;
    }

    function remainingSleepTimerDuration() {
        return playerLoader.item.remainingSleepTimerDuration();
    }

    property alias renderingModel: renderingModelLoader.item

    Loader {
        id: renderingModelLoader
        asynchronous: true
        sourceComponent: Component {
            RenderingModel {
            }
        }
    }

    Loader {
        id: playerLoader
        asynchronous: true
        sourceComponent: Component {
            ZonePlayer {
                onConnectedChanged: player.connected = connected
                onRenderingGroupChanged: player.refreshRenderingGroup()
                onRenderingChanged: player.refreshRendering()
                onSourceChanged: player.refreshSource()
                onPlayModeChanged: player.refreshPlayMode()
                onStatusChanged: {}
                onStopped: player.stopped()
                onSleepTimerChanged: player.sleepTimerEnabled = player.remainingSleepTimerDuration() > 0 ? true : false
            }
        }
    }

    property alias trackQueue: trackQueueLoader.item

    Loader {
        id: trackQueueLoader
        asynchronous: true
        sourceComponent: Component {
            TrackQueue {
            }
        }
    }

    Loader {
        id: playingTimerLoader
        asynchronous: true
        sourceComponent: Component {
            Timer {
                interval: 1000;
                running: false;
                repeat: true;
                onTriggered: {
                    var npos = player.position + interval;
                    player.position = npos > player.duration ? 0 : npos;
                }
            }
        }
    }
}
