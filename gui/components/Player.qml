/*
 * Copyright (C) 2016-2019
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
    property alias trackQueue: trackQueueLoader.item
    property alias renderingModel: renderingModelLoader.item
    property bool connected: false
    property string zoneId: ""
    property string zoneName: ""
    property string zoneShortName: ""
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
    property string playbackState: ""
    readonly property bool isPlaying: (playbackState === "PLAYING")
    property int trackPosition: 0
    property int trackDuration: 1
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

    signal jobFailed()
    signal stopped()
    signal sourceChanged()
    signal currentPositionChanged(int position, int duration)

    onCurrentCountChanged: queueInfo = queueOverviewString()
    onCurrentIndexChanged: queueInfo = queueOverviewString()

    function connectZonePlayer(zonePlayer) {
        connected = false; // force signal
        var zp = zone.handle;
        zp.disableMPRIS2();
        zone.handle = zonePlayer;
        zone.handle.enableMPRIS2();
        zone.pid = zonePlayer.pid;

        // execute all handlers to signal the changes
        player.handleZPConnectedChanged();
        player.handleZPSourceChanged();
        player.handleZPRenderingGroupChanged();
        player.handleZPRenderingChanged();
        player.handleZPPlayModeChanged();
        player.handleZPPlaybackStateChanged();

        // release the old zone player
        AllZonesModel.releasePlayer(zp);
    }

    function coordinatorName() {
        return zone.handle.coordinatorName();
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

    function ping() {
        return connected && zone.handle.ping();
    }

    function stop() {
        return zone.handle.stop();
    }

    function play() {
        return zone.handle.play();
    }

    function pause() {
        return zone.handle.pause();
    }

    function toggle() {
        if (isPlaying)
            return pause();
        else
            return play();
    }

    function playSource(modelItem) {
        return zone.handle.startPlaySource(modelItem.payload);
    }

    function playSong(modelItem) {
        return playSource(modelItem);
    }

    function previousSong(startPlaying) {
        return zone.handle.previous()
    }

    function nextSong(startPlaying, fromControls) {
        return zone.handle.next();
    }

    function toggleRepeat() {
        return zone.handle.toggleRepeat()
    }

    function toggleShuffle() {
        return zone.handle.toggleShuffle()
    }

    function seek(position) {
        if (player.canSeekInStream())
            return zone.handle.seekTime(Math.floor(position / 1000));
        return false;
    }

    function setSource(modelItem) {
        return zone.handle.setSource(modelItem.payload);
    }

    function setVolumeGroup(volume) {
        return zone.handle.setVolumeGroup(volume);
    }

    function setVolume(uuid, volume) {
        return zone.handle.setVolume(uuid, volume);
    }

    function setBass(value) {
        return zone.handle.setBass(value)
    }

    function setTreble(value) {
        return zone.handle.setTreble(value)
    }

    function toggleMuteGroup() {
        return zone.handle.toggleMute();
    }

    function toggleMute(uuid) {
        return zone.handle.toggleMute(uuid);
    }

    function toggleNightmode() {
        return zone.handle.toggleNightmode();
    }

    function toggleLoudness() {
        return zone.handle.toggleLoudness();
    }

    function playQueue(start) {
        return zone.handle.playQueue(start);
    }

    function seekTrack(nr) {
        return zone.handle.seekTrack(nr);
    }

    function addItemToQueue(modelItem, nr) {
        return zone.handle.addItemToQueue(modelItem.payload, nr);
    }

    function makeFilePictureLocalURL(filePath) {
        return zone.handle.makeFilePictureLocalURL(filePath);
    }

    function makeFileStreamItem(filePath, codec, title, album, author, duration, hasArt) {
        return zone.handle.makeFileStreamItem(filePath, codec, title, album, author, duration, hasArt);
    }

    function addMultipleItemsToQueue(modelItemList) {
        var payloads = [];
        for (var i = 0; i < modelItemList.length; ++i)
            payloads.push(modelItemList[i].payload);
        return zone.handle.addMultipleItemsToQueue(payloads);
    }

    function removeAllTracksFromQueue() {
        return zone.handle.removeAllTracksFromQueue();
    }

    function removeTrackFromQueue(modelItem) {
        return zone.handle.removeTrackFromQueue(modelItem.id, trackQueue.model.containerUpdateID());
    }

    function reorderTrackInQueue(nrFrom, nrTo) {
        return zone.handle.reorderTrackInQueue(nrFrom, nrTo, trackQueue.model.containerUpdateID());
    }

    function saveQueue(title) {
        return zone.handle.saveQueue(title);
    }

    function createSavedQueue(title) {
        return zone.handle.createSavedQueue(title);
    }

    function addItemToSavedQueue(playlistId, modelItem, containerUpdateID) {
        return zone.handle.addItemToSavedQueue(playlistId, modelItem.payload, containerUpdateID);
    }

    function removeTracksFromSavedQueue(playlistId, selectedIndices, containerUpdateID) {
        return zone.handle.removeTracksFromSavedQueue(playlistId, selectedIndices, containerUpdateID);
    }

    function reorderTrackInSavedQueue(playlistId, index, newIndex, containerUpdateID) {
        return zone.handle.reorderTrackInSavedQueue(playlistId, index, newIndex, containerUpdateID);
    }

    function configureSleepTimer(sec) {
        return zone.handle.configureSleepTimer(sec);
    }

    function remainingSleepTimerDuration() {
        return zone.handle.remainingSleepTimerDuration();
    }

    function playStream(url, title) {
        return zone.handle.startPlayStream(url, (title === "" ? qsTr("Untitled") : title))
    }

    function playLineIN() {
        return zone.handle.playLineIN()
    }

    function playDigitalIN() {
        return zone.handle.playDigitalIN()
    }

    function playPulse() {
        return zone.handle.playPulse()
    }

    function isPulseStream() {
        return zone.handle.isPulseStream(currentMetaSource);
    }

    function isMyStream(metaSource) {
        return zone.handle.isMyStream(metaSource);
    }

    function playFavorite(modelItem) {
        return zone.handle.startPlayFavorite(modelItem.payload);
    }

    function isPlayingQueued() {
        return player.trackDuration > 0;
    }

    function canSeekInStream() {
        switch (currentProtocol) {
        case 1:  // x-rincon-stream
        case 2:  // x-rincon-mp3radio
        case 5:  // x-sonos-htastream
        case 14: // http-get
            return false;
        default:
            // the noson streamer uses the protocol http(17)
            return isPlayingQueued();
        }
    }

    // reload the rendering model
    function refreshRendering() {
        handleZPRenderingChanged();
    }

    ////////////////////////////////////////////////////////////////////////////
    // handlers connected to the ZP events
    // they apply changes on the player properties and forward signals as needed
    //
    function handleZPConnectedChanged() {
        player.connected = zone.handle.connected;
        player.zoneId = zone.handle.zoneId;
        player.zoneName = zone.handle.zoneName;
        player.zoneShortName = zone.handle.zoneShortName;
        player.controllerURI = zone.handle.controllerURI;
        trackQueue.initQueue(zone.handle);
    }

    function handleZPSourceChanged() {
        // protect against undefined properties
        player.currentMetaAlbum = zone.handle.currentMetaAlbum || "";
        player.currentMetaArt = zone.handle.currentMetaArt || "";
        player.currentMetaArtist = zone.handle.currentMetaArtist || "";
        player.currentMetaSource = zone.handle.currentMetaSource || "";
        player.currentMetaTitle = zone.handle.currentMetaTitle || "";
        player.currentMetaURITitle = zone.handle.currentMetaURITitle || "";
        player.currentIndex = zone.handle.currentIndex;
        player.currentProtocol = zone.handle.currentProtocol;
        player.trackDuration = 1000 * zone.handle.currentTrackDuration;
        // reset position
        var npos = 1000 * zone.handle.currentTrackPosition();
        player.trackPosition = npos > player.trackDuration ? 0 : npos;

        if (player.currentProtocol == 1) {
            player.covers = [{art: "qrc:/images/linein.png"}];
        } else if (player.currentProtocol == 5) {
            player.covers = [{art: "qrc:/images/tv.png"}];
        } else {
            player.covers = makeCoverSource(player.currentMetaArt, player.currentMetaArtist, player.currentMetaAlbum);
        }
        player.sourceChanged();
        player.currentPositionChanged(player.trackPosition, player.trackDuration);
    }

    function handleZPPlayModeChanged() {
        player.repeat = zone.handle.playMode === "REPEAT_ALL" || zone.handle.playMode === "REPEAT_ONE" || zone.handle.playMode === "SHUFFLE" ? true : false;
        player.shuffle = zone.handle.playMode === "SHUFFLE" || zone.handle.playMode === "SHUFFLE_NOREPEAT" ? true : false;
    }

    function handleZPRenderingGroupChanged() {
        player.volumeMaster = zone.handle.volumeMaster;
        player.treble = zone.handle.treble;
        player.bass = zone.handle.bass;
        player.mute = zone.handle.muteMaster;
        player.nightmodeEnabled = zone.handle.nightmode;
        player.loudnessEnabled = zone.handle.loudness;
        player.outputFixed = zone.handle.outputFixed;
    }

    function handleZPRenderingChanged() {
        renderingModelLoader.item.load(zone.handle);
        renderingControlCount = renderingModelLoader.item.count;
    }

    function handleZPPlaybackStateChanged() {
        player.playbackState = zone.handle.playbackState;
        if (zone.handle.playbackState === "PLAYING" && player.currentIndex >= 0) {
            // Starting playback of queued track the position can be resetted without any event
            // from the SONOS device. This hack query to retrieve the real position after a short
            // time (3sec).
            syncPosition.start();
        } else if (zone.handle.playbackState === "STOPPED") {
            stopped();
        }
    }

    Timer {
        id: syncPosition
        interval: 3000
        onTriggered: {
            if (isPlaying) {
                var npos = 1000 * zone.handle.currentTrackPosition();
                player.trackPosition = npos > player.trackDuration ? 0 : npos;
                customdebug("sync position to " + player.trackPosition);
            }
        }
    }

    Connections {
        target: AllZonesModel
        onZpJobFailed: {
            if (pid === zone.pid) {
                popInfo.open(qsTr("Action can't be performed"));
                player.jobFailed();
            }
        }
        onZpConnectedChanged: { if (pid === zone.pid) handleZPConnectedChanged(); }
        onZpSourceChanged: { if (pid === zone.pid) handleZPSourceChanged(); }
        onZpRenderingGroupChanged: { if (pid === zone.pid) handleZPRenderingGroupChanged(); }
        onZpRenderingChanged: { if (pid === zone.pid) handleZPRenderingChanged(); }
        onZpPlayModeChanged: { if (pid === zone.pid) handleZPPlayModeChanged(); }
        onZpPlaybackStateChanged: { if (pid === zone.pid) handleZPPlaybackStateChanged(); }
        onZpSleepTimerChanged: {
            if (pid === zone.pid)
              player.sleepTimerEnabled = player.remainingSleepTimerDuration() > 0 ? true : false;
        }
    }

    QtObject {
        id: zone
        objectName: "playerZone"
        property int pid: 0
        property ZonePlayer handle: ZonePlayer { }
    }

    Loader {
        id: renderingModelLoader
        asynchronous: false
        sourceComponent: Component {
            RenderingModel {
            }
        }
    }

    // the track queue related to the player
    // it must be plugged to the current instance of the zone player
    // also it must be reloaded after any connection event
    Loader {
        id: trackQueueLoader
        asynchronous: false
        sourceComponent: Component {
            TrackQueue {
                // init is done by handleZPConnectedChanged
                onTrackCountChanged: player.currentCount = trackCount
            }
        }
    }

    Timer {
        id: playingTimer
        interval: 1000;
        running: (player.isPlaying && player.trackDuration > 1)
        repeat: true;
        onTriggered: {
            var npos = player.trackPosition + interval;
            player.trackPosition = npos > player.trackDuration ? 0 : npos;
            player.currentPositionChanged(player.trackPosition, player.trackDuration);
        }
    }
}
