/*
 * Copyright (C) 2019
 *      Adam Pigg <adam@piggz.co.uk>
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

import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"
import "components/Dialog"
import NosonApp 1.0
import NosonThumbnailer 1.0
import "components"
import "components/Dialog"
import Nemo.Configuration 1.0

ApplicationWindow {
    id: mainView

    MusicInfoBox {
        height: units.gu(4)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: Theme.horizontalPageMargin * 2
        anchors.right: parent.right
        anchors.rightMargin: Theme.horizontalPageMargin * 2
        visible: pageStack.currentPage.pageTitle !== undefined
    }

    initialPage: Component {
        MusicServices {
        }
    }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    BlurredBackground {
        id: blurredBackground
        anchors.fill: parent
        z: -1
    }

    ConfigurationGroup {
        id: settings
        path: "/uk/co/piggz/noson"
        synchronous: true
        
        property real scaleFactor: 1.0
        property real fontScaleFactor: 1.0
        property bool firstRun: true
        property string zoneName
        property string coordinatorName: ""
        property int tabIndex: -1
        property string accounts
        property string lastfmKey
        property string deviceUrl
    }
    
    StyleLight {
        id: styleMusic
    }
    
    Units {
        id: units
        scaleFactor: 1.0
        fontScaleFactor: 1.0
    }
    
    // The player handles all actions to control the music
    Player {
        id: player
    }
    
    PopInfo {
        id: popInfo
        backgroundColor: styleMusic.popover.backgroundColor
        labelColor: styleMusic.popover.labelColor
    }
    
    // Variables
    property string appName: "Noson"    // My name
    property int debugLevel: 2          // My debug level
    property bool playOnStart: false    // play inputStreamUrl when startup is completed
    property bool startup: true         // is running the cold startup ?
    property bool ssdp: true            // point out the connect method

    // Property to store the state of the application (active or suspended)
    property bool applicationSuspended: false

    // setting alias to check first run
    //property alias firstRun: settings.firstRun

    // setting alias to store deviceUrl as hint for the SSDP discovery
    property alias deviceUrl: settings.deviceUrl

    // setting alias to store last zone connected
    property alias currentZone: settings.zoneName
    property alias currentCoordinator: settings.coordinatorName
    property string currentZoneTag: ""

    // track latest stream link
    property string inputStreamUrl: ""

    // No source configured: First user has to select a source
    property bool noMusic: player.currentMetaSource === "" && loadedUI

    // No zone connected: UI push page "NoZoneState" on top to invit user to retry discovery of Sonos devices
    property bool noZone: false // doesn't pop page on startup
    property Page noZonePage

    // current page now playing
    property Page nowPlayingPage

    // property to detect if the UI has finished
    property bool loadedUI: false
    property real wideSongView: units.gu(70)
    property bool wideAspect: width >= units.gu(100) && loadedUI

    // property to enable pop info on index loaded
    property bool infoLoadedIndex: true // enabled at startup

    // property to detect thumbnailer is available
    property bool thumbValid: false

    // Constants
    readonly property int queueBatchSize: 100
    readonly property real minSizeGU: 42
    readonly property string tr_undefined: qsTr("<Undefined>")

    // built-in cache for genre artworks
    property var genreArtworks: []

    property bool alarmEnabled: false
    property bool shareIndexInProgress: false

    // about alarms
    AlarmsModel {
        id: alarmsModel
        property bool updatePending: false
        property bool dataSynced: false

        onDataUpdated: asyncLoad()

        onLoaded: {
            if (updatePending) {
                // delay model reset while a dialog still opened
                dataSynced = false;
            } else {
                resetModel();
                dataSynced = true;
            }
        }

        onUpdatePendingChanged: {
            if (!updatePending && !dataSynced) {
                resetModel();
                dataSynced = true;
            }
        }

        onCountChanged: alarmEnabled = isAlarmEnabled()
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Events
    ////

    Connections {
        target: Qt.application
        onStateChanged: {
            switch (Qt.application.state) {
            case Qt.ApplicationSuspended:
            case Qt.ApplicationInactive:
                if (!applicationSuspended) {
                    customdebug("Application state changed to suspended");
                    applicationSuspended = true;
                }
                break;
            case Qt.ApplicationHidden:
            case Qt.ApplicationActive:
                if (applicationSuspended) {
                    customdebug("Application state changed to active");
                    applicationSuspended = false;
                    if (!noZone) {
                        player.ping(function(result) {
                            if (result) {
                                customdebug("Renew all subscriptions");
                                var future = Sonos.tryRenewSubscriptions();
                                future.finished.connect(actionFinished);
                                future.start();
                                // resync track position after sleeping
                                if (player.isPlaying && player.trackDuration > 0)
                                    player.syncTrackPosition();
                            } else {
                                noZone = true;
                            }
                        });
                    }
                }
                break;
            }
        }
    }

    Connections {
        target: Sonos

        onJobCountChanged: jobRunning = Sonos.jobCount > 0 ? true : false

        onInitDone: {
            if (succeeded) {
                // clear the setting deviceUrl when ssdp method succeeded
                if (ssdp && deviceUrl !== "") {
                    customdebug("NOTICE: Clearing the configured URL because invalid");
                    deviceUrl = "";
                }
                if (noZone)
                    noZone = false;
            } else {
                if (!noZone)
                    noZone = true;
            }
        }

        onLoadingFinished: {
            if (infoLoadedIndex) {
                infoLoadedIndex = false;
                popInfo.open(qsTr("Index loaded"));
            }
        }

        onTopologyChanged: {
            AllZonesModel.asyncLoad();
        }
    }

    // Run on startup
    Component.onCompleted: {
        customdebug("LANG=" + Qt.locale().name);
        Sonos.setLocale(Qt.locale().name);

        // configure the thumbnailer
        if (settings.lastfmKey && settings.lastfmKey.length > 1) {
            if (Thumbnailer.configure("LASTFM", settings.lastfmKey))
                thumbValid = true;
        } else {
            if (Thumbnailer.configure("DEEZER", "n/a"))
                thumbValid = true;
        }

        // init SMAPI third party accounts
        var acls = deserializeACLS(settings.accounts);
        for (var i = 0; i < acls.length; ++i) {
            customdebug("register account: type=" + acls[i].type + " sn=" + acls[i].sn + " token=" + acls[i].token.substr(0, 1) + "...");
            Sonos.addServiceOAuth(acls[i].type, acls[i].sn, acls[i].key, acls[i].token, acls[i].username);
        }

        // initialize all data models
        AllZonesModel.init(Sonos, "",       false);
        AllFavoritesModel.init(Sonos, "",   false);
        AllServicesModel.init(Sonos,        false);
        AllPlaylistsModel.init(Sonos, "",   false);
        MyServicesModel.init(Sonos,         false);
        alarmsModel.init(Sonos,             false);
        // launch connection
        connectSonos();

        // signal UI has finished
        loadedUI = true;

    }

    // Show/hide page NoZoneState
    onNoZoneChanged: {
        if (noZone) {
            noZonePage = pageStack.push("pages/NoZoneState.qml")
        } else {
            if (pageStack.currentPage === noZonePage) {
                pageStack.pop()
            }
        }
    }

    // About backend signals
    // Triggers asynchronous loading on dataUpdated from global models
    // For these singletons, data loading is processed by backend threads.
    // Invoking asyncLoad() will schedule the reloading of data.
    Connections {
        target: AllZonesModel
        onDataUpdated: AllZonesModel.asyncLoad()
        onLoaded: {
            AllZonesModel.resetModel();
            reloadZone();
        }
    }

    Connections {
        target: AllServicesModel
        onDataUpdated: AllServicesModel.asyncLoad()
        onLoaded: AllServicesModel.resetModel()
    }

    Connections {
        target: MyServicesModel
        onDataUpdated: MyServicesModel.asyncLoad()
        onLoaded: MyServicesModel.resetModel()
    }

    Connections {
        target: AllFavoritesModel
        onDataUpdated: AllFavoritesModel.asyncLoad()
        onLoaded: AllFavoritesModel.resetModel()
        onCountChanged: { tabs.setProperty(2, "visible", (AllFavoritesModel.count > 0)) }
    }

    Connections {
        target: AllArtistsModel
        onDataUpdated: AllArtistsModel.asyncLoad()
        onLoaded: AllArtistsModel.resetModel()
    }

    Connections {
        target: AllAlbumsModel
        onDataUpdated: AllAlbumsModel.asyncLoad()
        onLoaded: AllAlbumsModel.resetModel()
    }

    Connections {
        target: AllGenresModel
        onDataUpdated: AllGenresModel.asyncLoad()
        onLoaded: AllGenresModel.resetModel()
    }

    Connections {
        target: AllComposersModel
        onDataUpdated: AllComposersModel.asyncLoad()
        onLoaded: AllComposersModel.resetModel()
    }

    Connections {
        target: AllPlaylistsModel
        onDataUpdated: AllPlaylistsModel.asyncLoad()
        onLoaded: AllPlaylistsModel.resetModel()
    }

    Connections {
        target: Sonos
        onAlarmClockChanged: alarmsModel.asyncLoad()
        onShareIndexInProgress: {
            if (!shareIndexInProgress) {
                shareIndexInProgress = true;
            }
        }
        onShareIndexFinished: {
            if (shareIndexInProgress) {
                shareIndexInProgress = false;
                // Queue item metadata could be outdated: force reloading of the queue
                player.trackQueue.loadQueue();
                // Force reload genres to be sure the items count is uptodate
                if (!AllGenresModel.isNew()) {
                    AllGenresModel.asyncLoad();
                }
            }
        }
    }

    onZoneChanged: {
        // check for enabled alarm
        alarmEnabled = isAlarmEnabled();
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global actions & helpers
    ////

    // Custom debug funtion that's easier to shut off
    function customdebug(text) {
        var debug = true
        // set to "0" for not debugging
        //if (args.values.debug) { // *USE LATER*
        if (debug) {
            console.info(text)
        }
    }

    // ACLS is array as [{type, sn, key, token}]
    function serializeACLS(acls) {
        var str = ""
        for (var i = 0; i < acls.length; ++i) {
            if (i > 0)
                str += "|"
            str += acls[i].type + "," + acls[i].sn + "," + Qt.btoa(
                        acls[i].key) + "," + Qt.btoa(
                        acls[i].token) + "," + Qt.btoa(acls[i].username)
        }
        return str
    }

    // str format look like 'type0,sn0,key0|type1,sn1,key1'
    function deserializeACLS(str) {
        var acls = []
        var rows = str.split("|")
        for (var r = 0; r < rows.length; ++r) {
            var attrs = rows[r].split(",")
            if (attrs.length === 4)
                acls.push({
                              "type": attrs[0],
                              "sn": attrs[1],
                              "key": Qt.atob(attrs[2]),
                              "token": Qt.atob(attrs[3]),
                              "username": ""
                          })
            else if (attrs.length === 5)
                acls.push({
                              "type": attrs[0],
                              "sn": attrs[1],
                              "key": Qt.atob(attrs[2]),
                              "token": Qt.atob(attrs[3]),
                              "username": Qt.atob(attrs[4])
                          })
        }
        return acls
    }

    // Try connect to SONOS system
    function connectSonos() {
        // if the setting deviceUrl is filled then try it, else continue with the SSDP discovery
        if (deviceUrl.length > 0) {
            customdebug("NOTICE: Connecting using the configured URL: " + deviceUrl);
            ssdp = false; // point out the ssdp discovery isn't used to connect
            if (Sonos.init(debugLevel, deviceUrl))
                return true;
            customdebug("ERROR: Connection has failed using the configured URL: " + deviceUrl);
        }
        ssdp = true; // point out the ssdp discovery is used to connect
        var future = Sonos.tryInit(debugLevel);
        future.finished.connect(actionFinished);
        return future.start();
    }

    function reloadZone() {
        customdebug("Reloading the zone ...");
        if (connectZone(currentZone)) {
            // launch the content loader thread
            Sonos.runLoader();
            // execute the requested actions at startup
            if (startup) {
                startup = false;
                if (playOnStart) {
                    player.playStream(inputStreamUrl, "", function(result) {
                        if (result) {
                            tabs.pushNowPlaying();
                        } else {
                            actionFailed();
                        }
                    });
                }
            }
        }
    }

    signal zoneChanged

    // Try to change zone
    // On success noZone is set to false
    function connectZone(name) {
        customdebug("Connecting zone '" + name + "'")
        if (AllZonesModel.count === 0) {
            if (!noZone)
                noZone = true;
            return true;
        }
        var found = false;
        var model = null;
        // search for the zone name
        for (var p = 0; p < AllZonesModel.count; ++p) {
            model = AllZonesModel.get(p);
            if (model.name === name) {
                found = true;
                break;
            }
        }
        if (!found) {
            // search for the coordinator name
            for (p = 0; p < AllZonesModel.count; ++p) {
                model = AllZonesModel.get(p);
                if (model.coordinatorName === currentCoordinator) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            p = 0; // get the first
            model = AllZonesModel.get(0);
        }
        player.connectZonePlayer(AllZonesModel.holdPlayer(p));
        currentZone = model.name;
        currentCoordinator = model.coordinatorName;
        currentZoneTag = model.shortName;
        zoneChanged();

        if (noZone)
            noZone = false;
        return true;
    }

    // default action on failure
    function actionFailed() {
        popInfo.open(qsTr("Action can't be performed"));
    }
    // default callback on signal finished action
    function actionFinished(result) {
        if (!result)
            actionFailed();
    }

    // Action on request to update music library
    function updateMusicIndex() {
        var future = Sonos.tryRefreshShareIndex();
        future.finished.connect(function(result) {
            if (result) {
                // enable info on loaded index
                infoLoadedIndex = true;
                popInfo.open(qsTr("Refreshing of index is running"));
            } else {
                actionFailed();
            }
        });
        return future.start();
    }

    // Action on track clicked
    function trackClicked(modelItem, play) {
        play = play === undefined ? true : play // default play to true
        var nr = player.trackQueue.model.count + 1
        // push back
        if (play) {
            return player.playQueue(false, function(result) {
                if (result) {
                    player.addItemToQueue(modelItem, nr, function(result) {
                        var nr = result;
                        if (nr > 0) {
                            popInfo.open(qsTr("song added"));
                            player.seekTrack(nr, function(result) {
                                if (result) {
                                    player.play(function(result) {
                                        if (result) {
                                            // Show the Now playing page and make sure the track is visible
                                            tabs.pushNowPlaying();
                                        } else {
                                            actionFailed();
                                        }
                                    });
                                } else {
                                    actionFailed();
                                }
                            });
                        } else {
                            actionFailed();
                        }
                    });
                } else {
                    actionFailed();
                }
            });
        } else {
            return player.addItemToQueue(modelItem, nr, function(result) {
                if (result > 0) {
                    popInfo.open(qsTr("song added"));
                } else {
                    actionFailed();
                }
            });
        }
    }

    // Action on track from queue clicked
    function indexQueueClicked(index) {
        if (player.currentIndex === index) {
            return player.toggle(actionFinished);
        } else {
            return player.playQueue(false, function(result) {
                if (result) {
                    player.seekTrack(index + 1, function(result) {
                        if (result) {
                            player.play(actionFinished);
                        } else {
                            actionFailed();
                        }
                    });
                } else {
                    actionFailed();
                }
            });
        }
    }

    // Action on shuffle button clicked
    function shuffleModel(model) {
        var now = new Date()
        var seed = now.getSeconds()
        var index = 0

        if (model.totalCount !== undefined) {
            index = Math.floor(model.totalCount * Math.random(seed))
            while (model.count < index && model.loadMore())
                ;
        } else {
            index = Math.floor(model.count * Math.random(seed))
        }
        if (index >= model.count)
            return false
        else if (player.isPlaying)
            return trackClicked(model.get(index), false)
        else
            return trackClicked(model.get(index), true) // play track
    }

    // Action add queue multiple items
    function addMultipleItemsToQueue(modelItemList) {
        return player.addMultipleItemsToQueue(modelItemList, function(result) {
            if (result > 0) {
                popInfo.open(qsTr("%n song(s) added", "", modelItemList.length));
            } else {
                actionFailed();
            }
        });
    }

    // Action on play all button clicked
    function playAll(modelItem) {
        // replace queue with the bundle item
        return player.removeAllTracksFromQueue(function(result) {
            if (result) {
                player.addItemToQueue(modelItem, 0, function(result) {
                    var nr = result;
                    if (nr > 0) {
                        popInfo.open(qsTr("song added"));
                        player.playQueue(false, function(result) {
                            if (result) {
                                player.seekTrack(nr, function(result) {
                                    if (result) {
                                        player.play(function(result) {
                                            if (result) {
                                                // Show the Now playing page and make sure the track is visible
                                                tabs.pushNowPlaying();
                                            } else {
                                                actionFailed();
                                            }
                                        });
                                    } else {
                                        actionFailed();
                                    }
                                });
                            } else {
                                actionFailed();
                            }
                        });
                    } else {
                        actionFailed();
                    }
                });
            } else {
                actionFailed();
            }
        });
    }

    // Action add queue
    function addQueue(modelItem) {
        var nr = player.trackQueue.model.count
        return player.addItemToQueue(modelItem, ++nr, function(result) {
            if (result > 0) {
                popInfo.open(qsTr("song added"));
            } else {
                actionFailed();
            }
        });
    }

    // Action delete all tracks from queue
    function removeAllTracksFromQueue() {
        return player.removeAllTracksFromQueue(function(result) {
            if (result) {
                popInfo.open(qsTr("Queue cleared"));
            } else {
                actionFailed();
            }
        });
    }

    // Action on remove queue track
    function removeTrackFromQueue(modelItem) {
        return player.removeTrackFromQueue(modelItem, actionFinished);
    }


    // Action on move queue item
    function reorderTrackInQueue(from, to) {
        if (from < to)
            ++to;
        return player.reorderTrackInQueue(from + 1, to + 1, actionFinished);
    }

    // Action on radio item clicked
    function radioClicked(modelItem) {
        return player.playSource(modelItem, function(result) {
            if (result)
                tabs.pushNowPlaying();
            else
                popInfo.open(qsTr("Action can't be performed"));
        });
    }

    // Action on save queue
    function saveQueue(title) {
        return player.saveQueue(title, actionFinished);
    }

    // Action on create playlist
    function createPlaylist(title) {
        return player.createSavedQueue(title, actionFinished);
    }

    // Action on append item to a playlist
    function addPlaylist(playlistId, modelItem, containerUpdateID) {
        return player.addItemToSavedQueue(playlistId, modelItem, containerUpdateID, function(result) {
            if (result > 0) {
                popInfo.open(qsTr("song added"));
            } else {
                actionFailed();
            }
        });
    }

    // Action on remove item from a playlist
    function removeTracksFromPlaylist(playlistId, selectedIndices, containerUpdateID, onFinished) {
        return player.removeTracksFromSavedQueue(playlistId, selectedIndices, containerUpdateID, onFinished);
    }

    // Action on move playlist item
    function reorderTrackInPlaylist(playlistId, from, to, containerUpdateID, onFinished) {
        return player.reorderTrackInSavedQueue(playlistId, from, to, containerUpdateID, onFinished);
    }

    // Action on remove a playlist
    function removePlaylist(itemId) {
        var future = Sonos.tryDestroySavedQueue(itemId);
        future.finished.connect(actionFinished);
        return future.start();
    }

    // Action on check item as favorite
    function addItemToFavorites(modelItem, description, artURI) {
        var future = Sonos.tryAddItemToFavorites(modelItem.payload, description, artURI);
        future.finished.connect(actionFinished);
        return future.start();
    }

    // Action on uncheck item from favorites
    function removeFromFavorites(itemPayload) {
        var id = AllFavoritesModel.findFavorite(itemPayload)
        if (id.length === 0) // no favorite
            return true;
        var future = Sonos.tryDestroyFavorite(id);
        future.finished.connect(actionFinished);
        return future.start();
    }

    // Helpers

    // Converts an duration in ms to a formated string ("minutes:seconds")
    function durationToString(duration) {
        var minutes = Math.floor(duration / 1000 / 60)
        var seconds = Math.floor(duration / 1000) % 60
        // Make sure that we never see "NaN:NaN"
        if (minutes.toString() == 'NaN')
            minutes = 0
        if (seconds.toString() == 'NaN')
            seconds = 0
        return minutes + ":" + (seconds < 10 ? "0" + seconds : seconds)
    }

    function hashValue(str, modulo) {
        var hash = 0, i, chr, len
        if (str.length === 0)
            return hash
        for (i = 0,len = str.length; i < len; i++) {
            chr = str.charCodeAt(i)
            hash = ((hash << 5) - hash) + chr
            hash |= 0 // Convert to 32bit integer
        }
        return Math.abs(hash) % modulo
    }

    // Make a normalized string from input for filtering
    function normalizedInput(str) {
        return Sonos.normalizedInputString(str)
    }

    // Make container item from model item
    function makeContainerItem(model) {
        return {
            "id": model.id,
            "payload": model.payload
        }
    }

    function makeArt(art, artist, album) {
        if (art !== undefined && art !== "")
            return art;
        if (album !== undefined && album !== "") {
            if (thumbValid)
                return "image://albumart/artist=" + encodeURIComponent(artist) + "&album=" + encodeURIComponent(album);
            else
                return "qrc:/images/no_cover.png";
        } else if (artist !== undefined && artist !== "") {
            if (thumbValid)
                return "image://artistart/artist=" + encodeURIComponent(artist);
            else
                return "qrc:/images/none.png";
        }
        return "qrc:/images/no_cover.png";
    }

    function makeCoverSource(art, artist, album) {
        var array = [];
        if (art !== undefined && art !== "")
            array.push( {art: art} );
        if (album !== undefined && album !== "") {
            if (thumbValid)
                array.push( {art: "image://albumart/artist=" + encodeURIComponent(artist) + "&album=" + encodeURIComponent(album)} );
            array.push( {art: "qrc:/images/no_cover.png"} );
        } else if (artist !== undefined && artist !== "") {
            if (thumbValid)
                array.push( {art: "image://artistart/artist=" + encodeURIComponent(artist)} );
            array.push( {art: "qrc:/images/none.png"} );
        } else {
            array.push( {art: "qrc:/images/no_cover.png"} );
        }
        return array;
    }

    function isAlarmEnabled() {
        var rooms = Sonos.getZoneRooms(player.zoneId);
        for (var i = 0; i < alarmsModel.count; ++i) {
            var alarm = alarmsModel.get(i)
            if (alarm.enabled) {
                for (var r = 0; r < rooms.length; ++r) {
                    if (rooms[r]['id'] === alarm.roomId) {
                        if (alarm.includeLinkedZones || rooms.length === 1)
                            return true
                    }
                }
            }
        }
        return false
    }

    //==============================================================
    // Pages

    ListModel {
        id: tabs
        ListElement { title: qsTr("My Index"); source: "pages/Index.qml"; visible: true }
        ListElement { title: qsTr("Favorites"); source: "pages/Favorites.qml"; visible: false }
        ListElement { title: qsTr("Playlists"); source: "pages/Playlists.qml"; visible: true }
        ListElement { title: qsTr("Alarm clock"); source: "pages/Alarms.qml"; visible: true }
        ListElement { title: qsTr("This Device"); source: "pages/ThisDevice.qml"; visible: true }

        function initialIndex() {
            return (settings.tabIndex === -1 ? 0
            : settings.tabIndex > tabs.count - 1 ? tabs.count - 1
            : settings.tabIndex);
        }

        function pushNowPlaying()
        {
            if (!wideAspect) {
                if (nowPlayingPage === null) {
                    pageStack.completeAnimation();
                    nowPlayingPage = pageStack.push("qrc:/sfos/pages/NowPlaying.qml");
                }
                if (nowPlayingPage.isListView) {
                    nowPlayingPage.isListView = false; // ensure full view
                }
            }
        }
        
        function pushPage(source) {
                pageStack.push(source);
        }
    }

    //==============================================================
    // Dialogues


    DialogApplicationSettings {
        id: dialogApplicationSettings
    }

    DialogAbout {
        id: dialogAbout
    }

    DialogSettings {
        id: dialogSettings
    }

    DialogSleepTimer {
        id: dialogSleepTimer
    }

    DialogSoundSettings {
        id: dialogSoundSettings
    }
    
    DialogNewPlaylist {
        id: dialogNewPlaylist
    }

    DialogManageQueue {
        id: dialogManageQueue
    }

    DialogSelectSource {
        id: dialogSelectSource
    }
    
    DialogSearchMusic {
        id: dialogSearch
    }

    DialogSongInfo {
        id: dialogSongInfo
    }

    DialogRemovePlaylist {
        id: dialogRemovePlaylist
    }
    
    DialogServiceLabel {
        id: dialogServiceLabel
    }

    DialogAlarm {
        id: dialogAlarm
        container: alarmsModel

        onOpened: {
            alarmsModel.updatePending = true;
        }

        onClosed: {
            alarmsModel.updatePending = false;
        }
    }


    //==============================================================
    // Spinner

    property bool jobRunning: false

    BusyIndicator {
        id: spinner
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        z: 100
        visible: jobRunning
        running: visible
    }
}
