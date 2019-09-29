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

import QtQuick 2.9
import NosonApp 1.0

/*
 * This file should *only* manage the media playing and the relevant settings
 * It should therefore only access ZonePlayer, trackQueue and Settings
 * Anything else within the app should use Connections to listen for changes
 */


Item {
    id: player
    objectName: "controller"
    property alias zonePlayer: playerLoader.item
    property bool connected: false
    property string controllerURI: ""
    property string currentMetaAlbum: ""
    property string currentMetaArt: ""
    property string currentMetaArtist: ""
    property string currentMetaSource: ""
    property string currentMetaTitle: ""
    property string currentMetaURITitle: ""
    property int currentProtocol: -1
    property int currentIndex: -1
    property int currentCount: 0
    property int duration: 1
    readonly property bool isPlaying: player.playbackState === "PLAYING"
    readonly property var playbackState: playerLoader.status == Loader.Ready ? playerLoader.item.playbackState : ""
    property int position: 0
    property int volumeMaster: 0
    property int bass: 0
    property int treble: 0
    property bool repeat: false
    property bool shuffle: false
    property bool mute: false
    property bool outputFixed: false
    property int renderingControlCount: 0
    property bool sleepTimerEnabled: false
    property bool nightmodeEnabled: false
    property bool loudnessEnabled: false
    property var covers: []

    property string queueInfo: queueOverviewString()

    signal stopped()
    signal sourceChanged()

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
                trackQueue.loadQueue();
                return true;
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
        if (playerLoader.item.ping()) {
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

    function playSource(modelItem) {
        return playerLoader.item.startPlaySource(modelItem.payload);
    }

    function playSong(modelItem) {
        return playSource(modelItem);
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
        if (player.canSeekInStream())
            return playerLoader.item.seekTime(Math.floor(position / 1000));
        return false;
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

    function setBass(value) {
        return playerLoader.item.setBass(value)
    }

    function setTreble(value) {
        return playerLoader.item.setTreble(value)
    }

    function toggleMuteGroup() {
        return playerLoader.item.toggleMute();
    }

    function toggleMute(uuid) {
        return playerLoader.item.toggleMute(uuid);
    }

    function toggleNightmode() {
        return playerLoader.item.toggleNightmode();
    }

    function toggleLoudness() {
        return playerLoader.item.toggleLoudness();
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

    function makeFilePictureLocalURL(filePath) {
        return playerLoader.item.makeFilePictureLocalURL(filePath);
    }

    function makeFileStreamItem(filePath, codec, title, album, author, duration, hasArt) {
        return playerLoader.item.makeFileStreamItem(filePath, codec, title, album, author, duration, hasArt);
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
        player.currentProtocol = playerLoader.item.currentProtocol;
        player.duration = 1000 * playerLoader.item.currentTrackDuration;
        // reset position
        var npos = 1000 * playerLoader.item.currentTrackPosition();
        player.position = npos > player.duration ? 0 : npos;

        if (player.currentProtocol == 1) {
            player.covers = [{art: "qrc:/images/linein.png"}];
        } else if (player.currentProtocol == 5) {
            player.covers = [{art: "qrc:/images/tv.png"}];
        } else {
            player.covers = makeCoverSource(player.currentMetaArt, player.currentMetaArtist, player.currentMetaAlbum);
        }

        player.sourceChanged();
    }

    function refreshPlayMode() {
        player.repeat = playerLoader.item.playMode === "REPEAT_ALL" || playerLoader.item.playMode === "SHUFFLE" ? true : false;
        player.shuffle = playerLoader.item.playMode === "SHUFFLE" || playerLoader.item.playMode === "SHUFFLE_NOREPEAT" ? true : false;
    }

    function refreshRenderingGroup() {
        player.volumeMaster = playerLoader.item.volumeMaster;
        player.treble = playerLoader.item.treble;
        player.bass = playerLoader.item.bass;
        player.mute = playerLoader.item.muteMaster;
        player.nightmodeEnabled = playerLoader.item.nightmode;
        player.loudnessEnabled = playerLoader.item.loudness;
        player.outputFixed = playerLoader.item.outputFixed;
    }

    function refreshRendering() {
        renderingModelLoader.item.load(playerLoader.item);
        renderingControlCount = renderingModelLoader.item.count;
    }

    function remainingSleepTimerDuration() {
        return playerLoader.item.remainingSleepTimerDuration();
    }

    function playStream(url, title) {
        return playerLoader.item.startPlayStream(url, (title === "" ? qsTr("Untitled") : title))
    }

    function playLineIN() {
        return playerLoader.item.playLineIN()
    }

    function playDigitalIN() {
        return playerLoader.item.playDigitalIN()
    }

    function playPulse() {
        return playerLoader.item.playPulse()
    }

    function isPulseStream() {
        return playerLoader.item.isPulseStream(currentMetaSource);
    }

    function isMyStream(metaSource) {
        return playerLoader.item.isMyStream(metaSource);
    }

    function addItemToFavorites(modelItem, description, artURI) {
        return playerLoader.item.addItemToFavorites(modelItem.payload, description, artURI);
    }

    function removeFavorite(itemId) {
        return playerLoader.item.destroyFavorite(itemId);
    }

    function playFavorite(modelItem) {
        return playerLoader.item.startPlayFavorite(modelItem.payload);
    }

    function isPlayingQueued() {
        return player.duration > 0;
    }

    function canSeekInStream() {
        switch (currentProtocol) {
        case 1:  // x-rincon-stream
        case 2:  // x-rincon-mp3radio
        case 5:  // x-sonos-htastream
        case 14: // http-get
        case 17: // http
            return false;
        default:
            return isPlayingQueued();
        }
    }

    property alias renderingModel: renderingModelLoader.item

    Loader {
        id: renderingModelLoader
        asynchronous: false
        sourceComponent: Component {
            RenderingModel {
            }
        }
    }

    Loader {
        id: playerLoader
        asynchronous: false
        sourceComponent: Component {
            ZonePlayer {
                onJobFailed: popInfo.open(qsTr("Action can't be performed"));
                onConnectedChanged: {
                    player.connected = connected;
                    player.controllerURI = controllerURI;
                }
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
        asynchronous: false
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
