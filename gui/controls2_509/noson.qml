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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Controls.Universal 2.2
import Qt.labs.settings 1.0
import QtGraphicalEffects 1.0
import NosonApp 1.0
import NosonThumbnailer 1.0
import "components"
import "components/Dialog"
import "../toolbox.js" as ToolBox

ApplicationWindow {
    id: mainView
    visible: true
    title: "noson"

    // Design stuff
    width: 360
    height: 640

    Settings {
        id: settings
        property string style: "Material"
        property int theme: 0

        property real scaleFactor: 1.0
        property real fontScaleFactor: 1.0
        property bool firstRun: true
        property string zoneName: ""
        property string coordinatorName: ""
        property int tabIndex: -1
        property int widthGU: Math.round(mainView.width / units.gridUnit)
        property int heightGU: Math.round(mainView.height / units.gridUnit)
        property string accounts: ""
        property string lastfmKey: ""
        property string deviceUrl: ""
        property string musicLocation: ""
        property bool preferListView: false
    }

    Material.accent: Material.Grey
    Universal.accent: "grey"

    //@FIXME: declare the property 'palette' that is missing in QtQuick.controls 2.2 (Qt-5.9)
    Item {
        id: palette
        property color base: {
            if (settings.style === "Material") {
                return Material.background
            } else if (settings.style === "Universal") {
                return Universal.background
            } else return "white"
        }
        property color text: {
            if (settings.style === "Material") {
                return Material.foreground
            } else if (settings.style === "Universal") {
                return Universal.foreground
            } else return "black"
        }
        property color highlight: "gray"
        property color shadow: "black"
        property color brightText: "dimgray"
        property color button: "darkgray"
        property color link: "green"
        property color toolTipBase: "black"
        property color toolTipText: "white"
    }

    StyleLight {
        id: styleMusic
    }

    Universal.theme: settings.theme
    Material.theme: settings.theme

    Units {
        id: units
        scaleFactor: settings.scaleFactor
        fontScaleFactor: settings.fontScaleFactor
    }

    PopInfo {
        id: popInfo
        backgroundColor: styleMusic.popover.backgroundColor
        labelColor: styleMusic.popover.labelColor
    }

    DialogSettings {
        id: dialogSonosSettings
    }

    Loader {
        id: zonesPageLoader
        asynchronous: true
        source: "qrc:/controls2/Zones.qml"
        visible: false
    }

    // The player handles all actions to control the music
    Player {
        id: player
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
    property alias firstRun: settings.firstRun

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

    // property to store index loading
    property bool indexLoaded: false

    // property to detect thumbnailer is available
    property bool thumbValid: false

    // Constants
    readonly property int queueBatchSize: 100
    readonly property real minSizeGU: 42
    readonly property string tr_undefined: qsTr("<Undefined>")

    minimumHeight: units.gu(minSizeGU)
    minimumWidth: units.gu(minSizeGU)

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
            if (Qt.application.state === Qt.ApplicationSuspended)
                applicationSuspended = true;
            else if (applicationSuspended === true) {
                applicationSuspended = false;
                if (!noZone) {
                    player.ping(function(result) {
                        if (result) {
                            customdebug("Renew all subscriptions");
                            var future = Sonos.tryRenewSubscriptions();
                            future.finished.connect(actionFinished);
                            future.start();
                        } else {
                            noZone = true;
                        }
                    });
                }
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
            if (!indexLoaded) {
                indexLoaded = true;
                popInfo.open(qsTr("Index loaded"));
            }
        }

        onTopologyChanged: {
            AllZonesModel.asyncLoad();
        }
    }

    // Run on startup
    Component.onCompleted: {
        var argno = 0;
        if (indexOfArgument("--debug") > 0) {
            mainView.debugLevel = 4;
        }
        // Argument --playurl={Stream URL}: Play URL at startup
        if ((argno = indexOfArgument("--playurl=")) > 0) {
            inputStreamUrl = ApplicationArguments[argno].slice(ApplicationArguments[argno].indexOf("=") + 1);
            playOnStart = (inputStreamUrl.length > 0);
            customdebug(argno + ": playurl=" + inputStreamUrl);
        }
        // Argument --zone={Zone name}: Connect to zone at startup
        if ((argno = indexOfArgument("--zone=")) > 0) {
            currentZone = ApplicationArguments[argno].slice(ApplicationArguments[argno].indexOf("=") + 1);
            customdebug(argno + ": zone=" + currentZone);
        }
        // Argument --deviceurl={http://host:port[/xml/device_description.xml]}: Hint for the SSDP discovery
        if ((argno = indexOfArgument("--deviceurl=")) > 0) {
            deviceUrl = ApplicationArguments[argno].slice(ApplicationArguments[argno].indexOf("=") + 1);
            customdebug(argno + ": deviceurl=" + deviceUrl);
        }

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
        AllZonesModel.init(Sonos, "");
        AllFavoritesModel.init(Sonos, "",   false);
        AllServicesModel.init(Sonos,        false);
        AllPlaylistsModel.init(Sonos, "",   false);
        MyServicesModel.init(Sonos,         false);
        alarmsModel.init(Sonos,             false);
        // launch connection
        connectSonos();

        // signal UI has finished
        loadedUI = true;

        // resize main view according to user settings
        if (!Android) {
            mainView.width = (settings.widthGU >= minSizeGU ? units.gu(settings.widthGU) : units.gu(minSizeGU));
            mainView.height = (settings.heightGU >= minSizeGU ? units.gu(settings.heightGU) : units.gu(minSizeGU));
        }
    }

    // Show/hide page NoZoneState
    onNoZoneChanged: {
        if (noZone) {
            noZonePage = stackView.push("qrc:/controls2/NoZoneState.qml")
        } else {
            if (stackView.currentItem === noZonePage) {
                stackView.pop()
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
                player.trackQueue.reloadQueue();
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

    // Find index of a command line argument else -1
    function indexOfArgument(argv) {
        for (var i = 0; i < ApplicationArguments.length; ++i) {
            if (ApplicationArguments[i].indexOf(argv) === 0)
                return i;
        }
        return -1;
    }

    // Custom debug funtion that's easier to shut off
    function customdebug(text) {
        var debug = true; // set to "0" for not debugging
        //if (args.values.debug) { // *USE LATER*
        if (debug) {
            console.info(text);
        }
    }

    // ACLS is array as [{type, sn, key, token}]
    function serializeACLS(acls) {
        var str = "";
        for (var i = 0; i < acls.length; ++i) {
            if (i > 0)
                str += "|";
            str += acls[i].type + "," + acls[i].sn + "," + Qt.btoa(acls[i].key) + "," + Qt.btoa(acls[i].token) + "," + Qt.btoa(acls[i].username);
        }
        return str;
    }

    // str format look like 'type0,sn0,key0|type1,sn1,key1'
    function deserializeACLS(str) {
        var acls = [];
        var rows = str.split("|");
        for (var r = 0; r < rows.length; ++r) {
            var attrs = rows[r].split(",");
            if (attrs.length === 4)
                acls.push({type: attrs[0], sn: attrs[1], key: Qt.atob(attrs[2]), token: Qt.atob(attrs[3]), username: ""});
            else if (attrs.length === 5)
                acls.push({type: attrs[0], sn: attrs[1], key: Qt.atob(attrs[2]), token: Qt.atob(attrs[3]), username: Qt.atob(attrs[4])});
        }
        return acls;
    }

    // Try connect to SONOS system
    function connectSonos() {
        // if the setting deviceUrl is filled then try it, else continue with the SSDP discovery
        if (deviceUrl.length > 0) {
            customdebug("NOTICE: Connecting using the configured URL: " + deviceUrl);
            ssdp = false; // point out the ssdp discovery isn't used to connect
            if (Sonos.init(debugLevel, deviceUrl)) {
                Sonos.renewSubscriptions();
                return true;
            }
            customdebug("ERROR: Connection has failed using the configured URL: " + deviceUrl);
        }
        ssdp = true; // point out the ssdp discovery is used to connect
        var future = Sonos.tryInit(debugLevel);
        future.finished.connect(function(result){
            if (result)
                Sonos.renewSubscriptions();
            else
                actionFailed();
        });
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
        customdebug("Connecting zone '" + name + "'");
        if (AllZonesModel.count === 0) {
            if (!noZone)
                noZone = true;
            return false;
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
                indexLoaded = false;
                popInfo.open(qsTr("Refreshing of index is running"));
            } else {
                actionFailed();
            }
        });
        return future.start();
    }

    // Action on track clicked
    function trackClicked(modelItem, play) {
        play = play === undefined ? true : play  // default play to true
        var nr = player.trackQueue.count + 1; // push back
        if (play) {
            return player.playQueue(false, function(result) {
                if (result) {
                    player.addItemToQueue(modelItem, nr, function(result) {
                        var nr = result;
                        if (nr > 0) {
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
    function shuffleModel(model)
    {
        var now = new Date();
        var seed = now.getSeconds();
        var index = 0;

        if (model.totalCount !== undefined) {
            index = Math.floor(model.totalCount * Math.random(seed));
            if (index < model.firstIndex || index >= model.firstIndex + model.count) {
                if (model.fetchAt(index - 1)) {
                    ToolBox.connectOnce(model.onViewUpdated, function(){
                        if (player.isPlaying)
                            return trackClicked(model.get(index - model.firstIndex), false);
                        else
                            return trackClicked(model.get(index - model.firstIndex), true); // play track
                    });
                }
            } else {
                if (player.isPlaying)
                    return trackClicked(model.get(index - model.firstIndex), false);
                else
                    return trackClicked(model.get(index - model.firstIndex), true); // play track
            }
        } else {
            index = Math.floor(model.count * Math.random(seed));
            if (index >= model.count)
                return false;
            else if (player.isPlaying)
                return trackClicked(model.get(index), false);
            else
                return trackClicked(model.get(index), true); // play track
        }
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
    function playAll(modelItem)
    {
        // replace queue with the bundle item
        return player.removeAllTracksFromQueue(function(result) {
            if (result) {
                player.addItemToQueue(modelItem, 0, function(result) {
                    var nr = result;
                    if (nr > 0) {
                        player.playQueue(false, function(result) {
                            if (result) {
                                player.seekTrack(nr, function(result) {
                                    if (result) {
                                        player.play(function(result) {
                                            if (result) {
                                                // Show the Now playing page and make sure the track is visible
                                                tabs.pushNowPlaying();
                                                popInfo.open(qsTr("song added"));
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
    function addQueue(modelItem)
    {
        var nr = player.trackQueue.count;
        return player.addItemToQueue(modelItem, ++nr, function(result) {
            if (result > 0) {
                popInfo.open(qsTr("song added"));
            } else {
                actionFailed();
            }
        });
    }

    // Action delete all tracks from queue
    function removeAllTracksFromQueue()
    {
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
        var minutes = Math.floor(duration / 1000 / 60);
        var seconds = Math.floor(duration / 1000) % 60;
        // Make sure that we never see "NaN:NaN"
        if (minutes.toString() === 'NaN')
            minutes = 0;
        if (seconds.toString() === 'NaN')
            seconds = 0;
        return minutes + ":" + (seconds<10 ? "0"+seconds : seconds);
    }

    function hashValue(str, modulo) {
        var hash = 0, i, chr, len;
        if (str.length === 0) return hash;
        for (i = 0, len = str.length; i < len; i++) {
          chr   = str.charCodeAt(i);
          hash  = ((hash << 5) - hash) + chr;
          hash |= 0; // Convert to 32bit integer
        }
        return Math.abs(hash) % modulo;
    }

    // Make a normalized string from input for filtering
    function normalizedInput(str) {
        return Sonos.normalizedInputString(str);
    }

    // Make container item from model item
    function makeContainerItem(model) {
        return {
            id: model.id,
            payload: model.payload
        };
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
            var alarm = alarmsModel.get(i);
            if (alarm.enabled) {
                for (var r = 0; r < rooms.length; ++r) {
                    if (rooms[r]['id'] === alarm.roomId) {
                        if (alarm.includeLinkedZones || rooms.length === 1)
                            return true;
                    }
                }
            }
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global keyboard shortcuts
    ////

    Shortcut {
        sequences: ["Esc", "Back"]
        enabled: noZone === false && stackView.depth > 1
        onActivated: {
            if (stackView.depth > 1) {
                if (stackView.currentItem.isRoot)
                    stackView.pop()
                else
                    stackView.currentItem.goUpClicked()
            }
        }
    }
    Shortcut {
        sequence: "Menu"
        onActivated: optionsMenu.open()
    }
    Shortcut {
        sequence: "Alt+Right"           // Alt+Right   Seek forward +10secs
        onActivated: {
            var position = player.trackPosition + 10000 < player.trackDuration
                ? player.trackPosition + 10000 : player.trackDuration;
            player.seek(position, actionFinished);
        }
    }
    Shortcut {
        sequence: "Alt+Left"            // Alt+Left    Seek backwards -10secs
        onActivated: {
            var position = player.trackPosition - 10000 > 0
                    ? player.trackPosition - 10000 : 0;
            player.seek(position, actionFinished);
        }
    }
    Shortcut {
        sequence: "Ctrl+Left"           // Ctrl+Left   Previous Song
        onActivated: {
            player.previousSong(actionFinished);
        }
    }
    Shortcut {
        sequence: "Ctrl+Right"          // Ctrl+Right  Next Song
        onActivated: {
            player.nextSong(actionFinished);
        }
    }
    Shortcut {
        sequence: "Ctrl+Up"             // Ctrl+Up     Volume up
        onActivated: {
            var v = player.volumeMaster + 5 > 100 ? 100 : player.volumeMaster + 5;
            player.setVolumeGroup(v, function(result) {
                if (result) {
                    player.volumeMaster = Math.round(v);
                } else {
                    actionFailed();
                }
            });
        }
    }
    Shortcut {
        sequence: "Ctrl+Down"           // Ctrl+Down   Volume down
        onActivated: {
            var v = player.volumeMaster - 5 < 0 ? 0 : player.volumeMaster - 5;
            player.setVolumeGroup(v, function(result) {
                if (result) {
                    player.volumeMaster = Math.round(v);
                } else {
                    actionFailed();
                }
            });
        }
    }
    Shortcut {
        sequence: "Ctrl+R"              // Ctrl+R       Repeat toggle
        onActivated: {
            var old = player.repeat
            player.toggleRepeat(function(result) {
                if (result) {
                    player.repeat = !old;
                } else {
                    actionFailed();
                }
            });
        }
    }
    Shortcut {
        sequence: "Ctrl+F"              // Ctrl+F      Show Search popup
        onActivated: {
            stackView.currentItem.searchClicked()
        }
    }
    Shortcut {
        sequence: "Ctrl+J"              // Ctrl+J      Jump to playing song
        onActivated: {
            tabs.pushNowPlaying();
            if (nowPlayingPage != null)
                nowPlayingPage.isListView = true;
        }
    }
    Shortcut {
        sequence: "Ctrl+N"              // Ctrl+N      Show Now playing
        onActivated: {
            tabs.pushNowPlaying()
        }
    }
    Shortcut {
        sequence: "Ctrl+P"              // Ctrl+P      Toggle playing state
        onActivated: {
            player.toggle(actionFinished);
        }
    }
    Shortcut {
        sequence: "Ctrl+Q"              // Ctrl+Q      Quit the app
        onActivated: {
            Qt.quit();
        }
    }
    Shortcut {
        sequence: "Ctrl+U"              // Ctrl+U      Shuffle toggle
        onActivated: {
            var old = player.shuffle
            player.toggleShuffle(function(result) {
                if (result) {
                    player.shuffle = !old;
                } else {
                    actionFailed();
                }
            });
        }
    }
    Shortcut {
        sequence: "P"                   // P          Toggle playing state
        onActivated: {
            player.toggle(actionFinished);
        }
    }
    Shortcut {
        sequence: "B"                   // B          Previous Song
        onActivated: {
            player.previousSong(true);
        }
    }
    Shortcut {
        sequence: "N"                   // N          Next Song
        onActivated: {
            player.nextSong(true, true);
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Application main view
    ////

    property bool alarmEnabled: false
    property bool shareIndexInProgress: false

    header: ToolBar {
        id: mainToolBar
        Material.foreground: styleMusic.view.foregroundColor
        Material.background: styleMusic.view.backgroundColor

        state: "default"
        states: [
            State {
                name: "default"
            }
        ]

        RowLayout {
            spacing: 0
            anchors.fill: parent

            Item {
                width: units.gu(6)
                height: width

                Icon {
                    width: units.gu(5)
                    height: width
                    anchors.centerIn: parent
                    source: {
                        if (mainView.noZone) {
                            "qrc:/images/info.svg"
                        } else if (stackView.depth > 1) {
                            if (stackView.currentItem.isRoot)
                                "qrc:/images/go-previous.svg"
                            else
                                "qrc:/images/go-up.svg"
                        } else {
                            "qrc:/images/navigation-menu.svg"
                        }
                    }

                    onClicked: {
                        if (stackView.depth > 1) {
                            if (stackView.currentItem.isRoot)
                                stackView.pop()
                            else
                                stackView.currentItem.goUpClicked()
                        } else {
                            drawer.open()
                        }
                    }

                    visible: !mainView.noZone
                    enabled: !mainView.jobRunning
                }
            }

            Label {
                id: titleLabel
                text: stackView.currentItem != null ? stackView.currentItem.pageTitle : ""
                font.pointSize: units.fs("x-large")
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true

                /* Show more info */
                Icon {
                    id: iconInfo
                    color: "#e95420"
                    width: units.gu(5)
                    height: width
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    source: shareIndexInProgress ? "qrc:/images/download.svg" : player.sleepTimerEnabled ? "qrc:/images/timer.svg" : "qrc:/images/alarm.svg"
                    visible: player.sleepTimerEnabled || alarmEnabled || shareIndexInProgress
                    enabled: visible
                    animationRunning: shareIndexInProgress
                }
            }

            Item {
                width: units.gu(6)
                height: width

                Icon {
                    width: units.gu(5)
                    height: width
                    anchors.centerIn: parent
                    source: "qrc:/images/home.svg"

                    onClicked: {
                        stackView.pop()
                    }

                    visible: (stackView.currentItem != null && !stackView.currentItem.isRoot)
                    enabled: visible
                }

                Icon {
                    width: units.gu(5)
                    height: width
                    anchors.centerIn: parent
                    source: "qrc:/images/contextual-menu.svg"

                    visible: (stackView.currentItem == null || stackView.currentItem.isRoot)
                    enabled: visible

                    onClicked: optionsMenu.open()

                    Menu {
                        id: optionsMenu
                        x: parent.width - width
                        transformOrigin: Menu.TopRight

                        MenuItem {
                            visible: !noZone
                            height: (visible ? implicitHeight : 0)
                            text: qsTr("Standby timer")
                            font.pointSize: units.fs("medium")
                            onTriggered: dialogSleepTimer.open()
                        }
                        MenuItem {
                            visible: !noZone
                            height: (visible ? implicitHeight : 0)
                            text: qsTr("Sonos settings")
                            font.pointSize: units.fs("medium")
                            onTriggered: dialogSonosSettings.open()
                        }
                        MenuItem {
                            text: qsTr("General settings")
                            font.pointSize: units.fs("medium")
                            onTriggered: dialogApplicationSettings.open()
                        }
                        MenuItem {
                            text: qsTr("About")
                            font.pointSize: units.fs("medium")
                            onTriggered: dialogAbout.open()
                        }
                    }
                }
            }
        }
    }

    ListModel {
        id: tabs
        ListElement { title: qsTr("My Services"); source: "qrc:/controls2/MusicServices.qml"; visible: true }
        ListElement { title: qsTr("My Index"); source: "qrc:/controls2/Index.qml"; visible: true }
        ListElement { title: qsTr("Favorites"); source: "qrc:/controls2/Favorites.qml"; visible: false }
        ListElement { title: qsTr("Playlists"); source: "qrc:/controls2/Playlists.qml"; visible: true }
        ListElement { title: qsTr("Alarm clock"); source: "qrc:/controls2/Alarms.qml"; visible: true }
        ListElement { title: qsTr("This Device"); source: "qrc:/controls2/ThisDevice.qml"; visible: true }

        function initialIndex() {
            return (settings.tabIndex === -1 ? 0
            : settings.tabIndex > tabs.count - 1 ? tabs.count - 1
            : settings.tabIndex);
        }

        function pushNowPlaying()
        {
            if (!wideAspect) {
                if (nowPlayingPage == null)
                    nowPlayingPage = stackView.push("qrc:/controls2/NowPlaying.qml", false, true);
                if (nowPlayingPage.isListView) {
                    nowPlayingPage.isListView = false; // ensure full view
                }
            }
        }
    }

    Drawer {
        id: drawer
        width: Math.min(mainView.width, mainView.height) / 2
        height: mainView.height
        interactive: stackView.depth === 1

        property alias currentIndex: pageList.currentIndex

        Component.onCompleted: {
            currentIndex = tabs.initialIndex();
            stackView.clear();
            stackView.push(pageList.model.get(currentIndex).source);
        }

        ListView {
            id: pageList

            focus: true
            currentIndex: -1
            anchors.fill: parent

            delegate: ItemDelegate {
                visible: model.visible
                height: model.visible ? implicitHeight : 0
                width: parent.width
                text: model.title
                font.pointSize: units.fs("large")
                highlighted: ListView.isCurrentItem
                onClicked: {
                    if (index !== pageList.currentIndex) {
                        stackView.clear(StackView.ReplaceTransition);
                        stackView.push(model.source);
                        pageList.currentIndex = index;
                        settings.tabIndex = index;
                    }
                    drawer.close()
                }
            }

            model: tabs

            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }

    property alias stackView: stackView

    StackView {
        id: stackView
        anchors {
            bottom: musicToolbar.top
            fill: undefined
            left: parent.left
            right: nowPlayingSidebarLoader.left
            top: parent.top
        }
        initialItem: "qrc:/controls2/Welcome.qml"
    }


    DialogApplicationSettings {
        id: dialogApplicationSettings
    }

    DialogAbout {
        id: dialogAbout
    }

    DialogManageQueue {
        id: dialogManageQueue
    }

    DialogSelectSource {
        id: dialogSelectSource
    }

    DialogSleepTimer {
        id: dialogSleepTimer
    }

    DialogSongInfo {
        id: dialogSongInfo
    }

    DialogSoundSettings {
        id: dialogSoundSettings
    }

    property alias musicToolbar: musicToolbar

    Loader {
        id: musicToolbar
        active: true
        height: units.gu(7.25)
        anchors { // start offscreen
            left: parent.left
            right: parent.right
            top: parent.bottom
            topMargin: shown ? -height : 0
        }
        asynchronous: true
        source: "qrc:/controls2/components/MusicToolbar.qml"

        property bool shown: status === Loader.Ready && !noZone && (!wideAspect || player.currentMetaSource === "") &&
                             (stackView.currentItem && (stackView.currentItem.showToolbar === undefined || stackView.currentItem.showToolbar))

    }

    Loader {
        id: nowPlayingSidebarLoader
        active: true
        width: units.gu(44)
        anchors {  // start offscreen
            bottom: parent.bottom
            left: parent.right
            top: parent.top
        }
        asynchronous: true
        source: "qrc:/controls2/components/NowPlayingSidebar.qml"
        anchors.leftMargin: shown ? -width : 0

        property bool shown: status === Loader.Ready && !noZone && loadedUI && wideAspect && player.currentMetaSource !== ""

        onShownChanged: {
            // move to current position in queue
            if (shown)
                item.activate();
            else
                item.deactivate();
        }

        Behavior on anchors.leftMargin {
            NumberAnimation {
            }
        }
    }

    property bool jobRunning: false

    ActivitySpinner {
        id: spinner
        visible: jobRunning
    }
}
