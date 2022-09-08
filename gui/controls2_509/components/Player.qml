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
    property alias trackQueue: queue
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
    property bool canGoNext: false
    property bool canGoPrevious: false
    property bool canSeek: false
    property bool currentInQueue: false
    property int currentIndex: -1   // index in queue
    property int currentTrackNo: -1 // track no
    property int currentCount: 0    // tracks count
    property string playbackState: ""
    readonly property bool isPlaying: (playbackState === "PLAYING")
    property int trackPosition: 0
    property int trackDuration: 0
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
    signal currentPositionChanged(int position, int duration)
    signal renderingControlChanged() // see function refreshRendering

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
        player.handleZPRenderingCountChanged();
        player.handleZPRenderingGroupChanged();
        player.handleZPRenderingChanged();
        player.handleZPPlayModeChanged();
        player.handleZPPlaybackStateChanged();

        // release the old zone player
        AllZonesModel.releasePlayer(zp);
    }

    function zoneHandle() {
        return (connected ? zone.handle : null);
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

    function ping(onFinished) {
        if (connected) {
            var future = zone.handle.tryPing();
            future.onFinished.connect(onFinished);
            return future.start();
        } else {
            onFinished(false);
        }
    }

    function stop(onFinished) {
        var future = zone.handle.tryStop();
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function play(onFinished) {
        var future = zone.handle.tryPlay();
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function pause(onFinished) {
        var future = zone.handle.tryPause();
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function toggle(onFinished) {
        if (isPlaying)
            return pause(onFinished);
        else
            return play(onFinished);
    }

    function playSource(modelItem, onFinished) {
        var future = zone.handle.tryPlaySource(modelItem.payload);
        // hack: run animation now without waiting event
        future.finished.connect(function(result) {
            if (!result)
                handleZPPlaybackStateChanged();
            onFinished(result);
        });
        if (future.start()) {
            player.playbackState = "TRANSITIONING";
            return true;
        }
        return false;
    }

    function previousSong(onFinished) {
        var future = zone.handle.tryPrevious();
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function nextSong(onFinished) {
        var future = zone.handle.tryNext();
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function toggleRepeat(onFinished) {
        var future = zone.handle.tryToggleRepeat();
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function toggleShuffle(onFinished) {
        var future = zone.handle.tryToggleShuffle();
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function seek(position, onFinished) {
        if (player.canSeek) {
            var future = zone.handle.trySeekTime(Math.floor(position / 1000));
            future.onFinished.connect(onFinished);
            return future.start();
        } else {
            onFinished(false);
        }
    }

    function setVolumeGroupForFake(volume) {
        return zone.handle.setVolumeGroup(volume, true);
    }

    function setVolumeForFake(uuid, volume) {
        return zone.handle.setVolume(uuid, volume, true);
    }

    function setVolumeGroup(volume, onFinished) {
        var future = zone.handle.trySetVolumeGroup(volume);
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function setVolume(uuid, volume, onFinished) {
        var future = zone.handle.trySetVolume(uuid, volume);
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function setBass(value, onFinished) {
        var future = zone.handle.trySetBass(value)
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function setTreble(value, onFinished) {
        var future = zone.handle.trySetTreble(value)
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function toggleMuteGroup(onFinished) {
        var future = zone.handle.tryToggleMute();
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function toggleMute(uuid, onFinished) {
        var future = zone.handle.tryToggleMute(uuid);
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function toggleNightmode(onFinished) {
        var future = zone.handle.tryToggleNightmode();
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function toggleLoudness(onFinished) {
        var future = zone.handle.tryToggleLoudness();
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function playQueue(start, onFinished) {
        var future = zone.handle.tryPlayQueue(start);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function seekTrack(nr, onFinished) {
        var future = zone.handle.trySeekTrack(nr);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function addItemToQueue(modelItem, nr, onFinished) {
        var future = zone.handle.tryAddItemToQueue(modelItem.payload, nr);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function makeFilePictureLocalURL(filePath) {
        return zone.handle.makeFilePictureLocalURL(filePath);
    }

    function makeFileStreamItem(filePath, codec, title, album, author, duration, hasArt) {
        return zone.handle.makeFileStreamItem(filePath, codec, title, album, author, duration, hasArt);
    }

    function addMultipleItemsToQueue(modelItemList, onFinished) {
        var payloads = [];
        for (var i = 0; i < modelItemList.length; ++i)
            payloads.push(modelItemList[i].payload);
        var future = zone.handle.tryAddMultipleItemsToQueue(payloads);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function removeAllTracksFromQueue(onFinished) {
        var future = zone.handle.tryRemoveAllTracksFromQueue();
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function removeTrackFromQueue(modelItem, onFinished) {
        var future = zone.handle.tryRemoveTrackFromQueue(modelItem.id, queue.containerUpdateID());
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function reorderTrackInQueue(nrFrom, nrTo, onFinished) {
        var future = zone.handle.tryReorderTrackInQueue(nrFrom, nrTo, queue.containerUpdateID());
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function saveQueue(title, onFinished) {
        var future = zone.handle.trySaveQueue(title);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function createSavedQueue(title, onFinished) {
        var future = zone.handle.tryCreateSavedQueue(title);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function addItemToSavedQueue(playlistId, modelItem, containerUpdateID, onFinished) {
        var future = zone.handle.tryAddItemToSavedQueue(playlistId, modelItem.payload, containerUpdateID);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function addMultipleItemsToSavedQueue(playlistId, payloads, containerUpdateID, onFinished) {
        var future = zone.handle.tryAddMultipleItemsToSavedQueue(playlistId, payloads, containerUpdateID);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function removeTracksFromSavedQueue(playlistId, selectedIndices, containerUpdateID, onFinished) {
        var future = zone.handle.tryRemoveTracksFromSavedQueue(playlistId, selectedIndices, containerUpdateID);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function reorderTrackInSavedQueue(playlistId, index, newIndex, containerUpdateID, onFinished) {
        var future = zone.handle.tryReorderTrackInSavedQueue(playlistId, index, newIndex, containerUpdateID);
        future.onFinished.connect(onFinished);
        return future.start();
    }

    function configureSleepTimer(sec, onFinished) {
        var future = zone.handle.tryConfigureSleepTimer(sec);
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function remainingSleepTimerDuration(onFinished) {
        var future = zone.handle.tryRemainingSleepTimerDuration();
        future.onFinished.connect(onFinished);
        return future.start(false);
    }

    function playStream(url, title, onFinished) {
        var future = zone.handle.tryPlayStream(url, (title === "" ? qsTr("Untitled") : title));
        // hack: run animation now without waiting event
        future.finished.connect(function(result) {
            if (!result)
                handleZPPlaybackStateChanged();
            onFinished(result);
        });
        if (future.start()) {
            player.playbackState = "TRANSITIONING";
            return true;
        }
        return false;
    }

    function playLineIN(onFinished) {
        var future = zone.handle.tryPlayLineIN();
        future.finished.connect(onFinished);
        return future.start();
    }

    function playDigitalIN(onFinished) {
        var future = zone.handle.tryPlayDigitalIN();
        future.finished.connect(onFinished);
        return future.start();
    }

    function playPulse(onFinished) {
        var future = zone.handle.tryPlayPulse();
        future.finished.connect(onFinished);
        return future.start();
    }

    function isPulseStream() {
        return zone.handle.isPulseStream(currentMetaSource);
    }

    function isMyStream(metaSource) {
        return zone.handle.isMyStream(metaSource);
    }

    function playFavorite(modelItem, onFinished) {
        var future = zone.handle.tryPlayFavorite(modelItem.payload);
        // hack: run animation now without waiting event
        future.finished.connect(function(result) {
            if (!result)
                handleZPPlaybackStateChanged();
            onFinished(result);
        });
        if (future.start()) {
            player.playbackState = "TRANSITIONING";
            return true;
        }
        return false;
    }

    function syncTrackPosition() {
        var future = zone.handle.tryCurrentTrackPosition();
        future.finished.connect(function(result) {
            var npos = (result > 0 ? 1000 * result : 0);
            player.trackPosition = npos > player.trackDuration ? 0 : npos;
            //customdebug("sync position to " + player.trackPosition);
        });
        future.start(false);
    }

    // reload the rendering model
    // it should be triggered on signal renderingControlChanged
    function refreshRendering() {
        renderingModelLoader.item.load(zone.handle);
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
        player.remainingSleepTimerDuration(function(result) {
            player.sleepTimerEnabled = result > 0 ? true : false
        });
        queue.initQueue(zone.handle);
    }

    function handleZPSourceChanged() {
        // protect against undefined properties
        player.currentMetaAlbum = zone.handle.currentMetaAlbum || "";
        player.currentMetaArt = zone.handle.currentMetaArt || "";
        player.currentMetaArtist = zone.handle.currentMetaArtist || "";
        player.currentMetaSource = zone.handle.currentMetaSource || "";
        player.currentMetaTitle = zone.handle.currentMetaTitle || "";
        player.currentMetaURITitle = zone.handle.currentMetaURITitle || "";
        player.currentProtocol = zone.handle.currentProtocol;
        player.canGoNext = zone.handle.canGoNext;
        player.canGoPrevious = zone.handle.canGoPrevious;
        player.canSeek = zone.handle.canSeek;
        player.currentInQueue = zone.handle.currentInQueue;
        if (zone.handle.currentInQueue)
            player.currentIndex = zone.handle.currentIndex;
        else
            player.currentIndex = -1;
        player.currentTrackNo = zone.handle.currentIndex;
        player.trackDuration = 1000 * zone.handle.currentTrackDuration;
        if (zone.handle.currentTrackDuration > 0) {
            player.syncTrackPosition();
        } else {
            player.trackPosition = 0;
        }
        // reset cover
        if (player.currentProtocol == 1) {
            player.covers = [{art: "qrc:/images/linein.png"}];
        } else if (player.currentProtocol == 5) {
            player.covers = [{art: "qrc:/images/tv.png"}];
        } else if (player.currentMetaArt != "") {
            player.covers = makeCoverSource(player.currentMetaArt, player.currentMetaArtist, player.currentMetaAlbum);
        } else if (player.currentMetaAlbum != "") {
            player.covers = makeCoverSource("", player.currentMetaArtist, player.currentMetaAlbum);
        } else {
            if (player.currentProtocol == 2)
                player.covers = [{art: "qrc:/images/radio.png"}];
            else
                player.covers = [];
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
        renderingControlChanged();
    }

    function handleZPRenderingCountChanged() {
        player.renderingControlCount = zone.handle.renderingCount;
    }

    function handleZPPlaybackStateChanged() {
        player.playbackState = zone.handle.playbackState;
        if (zone.handle.playbackState === "PLAYING" && zone.handle.currentIndex >= 0 &&
                zone.handle.currentTrackDuration > 0) {
            // Starting playback of queued track the position can be resetted without any event
            // from the SONOS device. This hack query to retrieve the real position after a short
            // time (3sec).
            delaySyncTrackPosition.start();
        } else if (zone.handle.playbackState === "STOPPED") {
            stopped();
        }
    }

    Timer {
        id: delaySyncTrackPosition
        interval: 3000
        onTriggered: {
            if (isPlaying && trackDuration > 0) {
                syncTrackPosition();
            }
        }
    }

    Connections {
        target: AllZonesModel
        onZpConnectedChanged: { if (pid === zone.pid) handleZPConnectedChanged(); }
        onZpSourceChanged: { if (pid === zone.pid) handleZPSourceChanged(); }
        onZpRenderingCountChanged: { if (pid === zone.pid) handleZPRenderingCountChanged(); }
        onZpRenderingGroupChanged: { if (pid === zone.pid) handleZPRenderingGroupChanged(); }
        onZpRenderingChanged: { if (pid === zone.pid) handleZPRenderingChanged(); }
        onZpPlayModeChanged: { if (pid === zone.pid) handleZPPlayModeChanged(); }
        onZpPlaybackStateChanged: { if (pid === zone.pid) handleZPPlaybackStateChanged(); }
        onZpSleepTimerChanged: {
            if (pid === zone.pid) {
                player.remainingSleepTimerDuration(function(result) {
                    player.sleepTimerEnabled = result > 0 ? true : false
                });
            }
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
    // so init is done by handleZPConnectedChanged
    QueueModel {
        id: queue
        onDataUpdated: {
            asyncLoad();
        }
        onLoaded: {
            if (succeeded) {
                resetModel();
                player.currentCount = totalCount;
            } else {
                resetModel();
                player.currentCount = 0;
            }
        }

        // Initialize the queue for given zone handle
        function initQueue(handle) {
            if (handle) {
                init(handle, false);
                asyncLoad();
            }
        }
        function reloadQueue() {
            asyncLoad();
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
