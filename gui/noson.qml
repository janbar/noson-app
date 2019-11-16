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
import "ui"

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
        property int tabIndex: -1
        property int widthGU: Math.round(mainView.width / units.gridUnit)
        property int heightGU: Math.round(mainView.height / units.gridUnit)
        property string accounts: ""
        property string lastfmKey: ""
        property string deviceUrl: ""
    }

    Material.accent: Material.Grey
    Universal.accent: Universal.Steel

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
        source: "qrc:/ui/Zones.qml"
        visible: false
    }

    // The player handles all actions to control the music
    Player {
        id: player
    }

    onApplicationSuspendedChanged: {
        if (!noZone && !applicationSuspended && player.connected) {
            mainView.jobRunning = true
            delayPlayerWakeUp.start()
        }
    }

    Timer {
        id: delayPlayerWakeUp
        interval: 100
        onTriggered: {
            if (!player.wakeUp())
                noZone = true
            else {
                Sonos.renewSubscriptions()
                noZone = false
            }
            mainView.jobRunning = false
        }
    }

    // Variables
    property string appName: "Noson"    // My name
    property int debugLevel: 2          // My debug level
    property bool playOnStart: false    // play inputStreamUrl when startup is completed
    property bool startup: true         // is running the cold startup ?
    property bool ssdp: true            // point out the connect method

    // Property to store the state of an application (active or suspended)
    property bool applicationSuspended: Qt.application.state === Qt.ApplicationSuspended

    // setting alias to check first run
    property alias firstRun: settings.firstRun

    // setting alias to store deviceUrl as hint for the SSDP discovery
    property alias deviceUrl: settings.deviceUrl

    // setting alias to store last zone connected
    property alias currentZone: settings.zoneName
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

    minimumHeight: units.gu(minSizeGU)
    minimumWidth: units.gu(minSizeGU)

    // built-in cache for genre artworks
    property var genreArtworks: []

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
            jobRunning = true;
            delayReloadZone.start();
        }
    }

    Timer {
        id: delayReloadZone
        interval: 100
        onTriggered: {
            customdebug("Reloading the zone ...");
            AllZonesModel.loadData();
            if (connectZone(currentZone)) {
                // launch the content loader thread
                Sonos.runLoader();
                // execute the requested actions at startup
                if (startup) {
                    startup = false;
                    if (playOnStart) {
                        if (player.playStream(inputStreamUrl, "")) {
                            tabs.pushNowPlaying();
                        }
                    }
                }
            } else {
                jobRunning = false;
            }
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

        // resize main view according to user settings
        if (!Android) {
            mainView.width = (settings.widthGU >= minSizeGU ? units.gu(settings.widthGU) : units.gu(minSizeGU));
            mainView.height = (settings.heightGU >= minSizeGU ? units.gu(settings.heightGU) : units.gu(minSizeGU));
        }
    }

    // Show/hide page NoZoneState
    onNoZoneChanged: {
        if (noZone) {
            noZonePage = stackView.push("qrc:/ui/NoZoneState.qml")
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
        onLoaded: AllZonesModel.resetModel()
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
            shareIndexInProgress = true;
            AllAlbumsModel.clearModel();
            AllArtistsModel.clearModel();
            AllComposersModel.clearModel();
            AllGenresModel.clearModel();
        }
        onShareIndexFinished: {
            shareIndexInProgress = false;
            // Queue item metadata could be outdated: force reloading of the queue
            player.trackQueue.loadQueue();
            // Force reload genres to be sure the items count is uptodate
            if (!AllGenresModel.isNew()) {
                AllGenresModel.asyncLoad();
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
            if (Sonos.init(debugLevel, deviceUrl))
                return true;
            customdebug("ERROR: Connection has failed using the configured URL: " + deviceUrl);
        }
        ssdp = true; // point out the ssdp discovery is used to connect
        return Sonos.startInit(debugLevel);
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
                if (model.coordinatorName === name) {
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
        currentZoneTag = model.shortName;
        zoneChanged();

        if (noZone)
            noZone = false;
        return true;
    }

    // Action on request to update music library
    function updateMusicIndex() {
        if (Sonos.refreshShareIndex()) {
            // enable info on loaded index
            infoLoadedIndex = true;
            popInfo.open(qsTr("Refreshing of index is running"));
            return true;
        }
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on track clicked
    function trackClicked(modelItem, play) {
        play = play === undefined ? true : play  // default play to true
        var nr = player.trackQueue.model.count + 1; // push back
        if (play) {
            if (player.playQueue(false))
            {
                nr = player.addItemToQueue(modelItem, nr);
                if (player.seekTrack(nr) && player.play())
                {
                    // Show the Now playing page and make sure the track is visible
                    tabs.pushNowPlaying();
                    return true;
                }
            }
            popInfo.open(qsTr("Action can't be performed"));
            return false;
        }
        else {
            if (player.addItemToQueue(modelItem, nr)) {
                popInfo.open(qsTr("song added"));
                return true;
            }
            popInfo.open(qsTr("Action can't be performed"));
            return false;
        }
    }

    // Action on track from queue clicked
    function indexQueueClicked(index) {
        if (player.currentIndex === index) {
            if (player.toggle())
                return true;
            popInfo.open(qsTr("Action can't be performed"));
            return false;
        }
        else if (player.playQueue(false) && player.seekTrack(index + 1) && player.play())
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on shuffle button clicked
    function shuffleModel(model)
    {
        var now = new Date();
        var seed = now.getSeconds();
        var index = 0;

        if (model.totalCount !== undefined) {
            index = Math.floor(model.totalCount * Math.random(seed));
            while (model.count < index && model.loadMore());
        }
        else {
            index = Math.floor(model.count * Math.random(seed));
        }
        if (index >= model.count)
            return false;
        else if (player.isPlaying)
            return trackClicked(model.get(index), false);
        else
            return trackClicked(model.get(index), true); // play track
    }

    // Action add queue multiple items
    function addMultipleItemsToQueue(modelItemList) {
        if (player.addMultipleItemsToQueue(modelItemList)) {
            popInfo.open(qsTr("%n song(s) added", "", modelItemList.length));
            return true;
        }
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on play all button clicked
    function playAll(modelItem)
    {
        // replace queue with the bundle item
        if (player.removeAllTracksFromQueue()) {
            var nr = player.addItemToQueue(modelItem, 0);
            if (nr && player.playQueue(false) && player.seekTrack(nr) && player.play()) {
                // Show the Now playing page and make sure the track is visible
                tabs.pushNowPlaying();
                popInfo.open(qsTr("song added"));
                return true;
            }
        }
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action add queue
    function addQueue(modelItem)
    {
        var nr = player.trackQueue.model.count;
        if (player.addItemToQueue(modelItem, ++nr) > 0) {
            popInfo.open(qsTr("song added"));
            return true;
        }
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action delete all tracks from queue
    function removeAllTracksFromQueue()
    {
        if (player.removeAllTracksFromQueue()) {
            popInfo.open(qsTr("Queue cleared"));
            return true;
        }
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on remove queue track
    function removeTrackFromQueue(modelItem) {
        if (player.removeTrackFromQueue(modelItem))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on move queue item
    function reorderTrackInQueue(from, to) {
        if (from < to)
            ++to;
        if (player.reorderTrackInQueue(from + 1, to + 1))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on radio item clicked
    function radioClicked(modelItem) {
        if (player.playSource(modelItem)) {
            tabs.pushNowPlaying();
            return true;
        }
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on save queue
    function saveQueue(title) {
        if (player.saveQueue(title))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on create playlist
    function createPlaylist(title) {
        if (player.createSavedQueue(title))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on append item to a playlist
    function addPlaylist(playlistId, modelItem, containerUpdateID) {
        if (player.addItemToSavedQueue(playlistId, modelItem, containerUpdateID)) {
            popInfo.open(qsTr("song added"));
            return true;
        }
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on remove item from a playlist
    function removeTracksFromPlaylist(playlistId, selectedIndices, containerUpdateID) {
        if (player.removeTracksFromSavedQueue(playlistId, selectedIndices, containerUpdateID)) {
            popInfo.open(qsTr("%n song(s) removed", "", selectedIndices.length));
            return true;
        }
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on move playlist item
    function reorderTrackInPlaylist(playlistId, from, to, containerUpdateID) {
        if (player.reorderTrackInSavedQueue(playlistId, from, to, containerUpdateID))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on remove a playlist
    function removePlaylist(itemId) {
        if (Sonos.destroySavedQueue(itemId))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on check item as favorite
    function addItemToFavorites(modelItem, description, artURI) {
        if (Sonos.addItemToFavorites(modelItem.payload, description, artURI))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Action on uncheck item from favorites
    function removeFromFavorites(itemPayload) {
        var id = AllFavoritesModel.findFavorite(itemPayload)
        if (id.length === 0) // no favorite
            return true;
        if (Sonos.destroyFavorite(id))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    // Helpers

    // Converts an duration in ms to a formated string ("minutes:seconds")
    function durationToString(duration) {
        var minutes = Math.floor(duration / 1000 / 60);
        var seconds = Math.floor(duration / 1000) % 60;
        // Make sure that we never see "NaN:NaN"
        if (minutes.toString() == 'NaN')
            minutes = 0;
        if (seconds.toString() == 'NaN')
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
            player.seek(position);
        }
    }
    Shortcut {
        sequence: "Alt+Left"            // Alt+Left    Seek backwards -10secs
        onActivated: {
            var position = player.trackPosition - 10000 > 0
                    ? player.trackPosition - 10000 : 0;
            player.seek(position);
        }
    }
    Shortcut {
        sequence: "Ctrl+Left"           // Ctrl+Left   Previous Song
        onActivated: {
            player.previousSong(true);
        }
    }
    Shortcut {
        sequence: "Ctrl+Right"          // Ctrl+Right  Next Song
        onActivated: {
            player.nextSong(true, true);
        }
    }
    Shortcut {
        sequence: "Ctrl+Up"             // Ctrl+Up     Volume up
        onActivated: {
            var v = player.volumeMaster + 5 > 100 ? 100 : player.volumeMaster + 5;
            if (player.setVolumeGroup(v))
                player.volumeMaster = Math.round(v);
        }
    }
    Shortcut {
        sequence: "Ctrl+Down"           // Ctrl+Down   Volume down
        onActivated: {
            var v = player.volumeMaster - 5 < 0 ? 0 : player.volumeMaster - 5;
            if (player.setVolumeGroup(v))
                player.volumeMaster = Math.round(v);
        }
    }
    Shortcut {
        sequence: "Ctrl+R"              // Ctrl+R       Repeat toggle
        onActivated: {
            var old = player.repeat
            if (player.toggleRepeat())
                player.repeat = !old
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
            if (nowPlayingPage !== null)
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
            player.toggle();
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
            if (player.toggleShuffle())
                player.shuffle = !old
        }
    }
    Shortcut {
        sequence: "P"                   // P          Toggle playing state
        onActivated: {
            player.toggle();
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
                    source: shareIndexInProgress ? "qrc:/images/sync.svg" : player.sleepTimerEnabled ? "qrc:/images/timer.svg" : "qrc:/images/alarm.svg"
                    visible: player.sleepTimerEnabled || alarmEnabled || shareIndexInProgress
                    enabled: visible
                    rotationRunning: shareIndexInProgress
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

                    visible: (stackView.currentItem !== null && !stackView.currentItem.isRoot)
                    enabled: visible
                }

                Icon {
                    width: units.gu(5)
                    height: width
                    anchors.centerIn: parent
                    source: "qrc:/images/contextual-menu.svg"

                    visible: (stackView.currentItem === null || stackView.currentItem.isRoot)
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
        ListElement { title: qsTr("My Services"); source: "qrc:/ui/MusicServices.qml"; visible: true }
        ListElement { title: qsTr("My Index"); source: "qrc:/ui/Index.qml"; visible: true }
        ListElement { title: qsTr("Favorites"); source: "qrc:/ui/Favorites.qml"; visible: false }
        ListElement { title: qsTr("Playlists"); source: "qrc:/ui/Playlists.qml"; visible: true }
        ListElement { title: qsTr("Alarm clock"); source: "qrc:/ui/Alarms.qml"; visible: true }
        ListElement { title: qsTr("This Device"); source: "qrc:/ui/ThisDevice.qml"; visible: true }

        function initialIndex() {
            return (settings.tabIndex === -1 ? 0
            : settings.tabIndex > tabs.count - 1 ? tabs.count - 1
            : settings.tabIndex);
        }

        function pushNowPlaying()
        {
            if (!wideAspect) {
                if (nowPlayingPage === null)
                    nowPlayingPage = stackView.push("qrc:/ui/NowPlaying.qml", false, true);
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
                    if (index != pageList.currentIndex) {
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
        initialItem: "qrc:/ui/Welcome.qml"
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
        anchors {
            left: parent.left
            right: parent.right
            top: parent.bottom
            topMargin: visible && status === Loader.Ready ? -height : 0
        }
        asynchronous: true
        source: "qrc:/components/MusicToolbar.qml"
        visible: !noZone && (!wideAspect || player.currentMetaSource === "") && status === Loader.Ready &&
                 (stackView.currentItem && (stackView.currentItem.showToolbar || stackView.currentItem.showToolbar === undefined))
    }

    Loader {
        id: nowPlayingSidebarLoader
        active: shown
        anchors {  // start offscreen
            bottom: parent.bottom
            right: parent.right
            top: parent.top
        }
        asynchronous: false
        source: "qrc:/components/NowPlayingSidebar.qml"
        width: shown && status === Loader.Ready ? units.gu(minSizeGU) : 0
        visible: width > 0

        property bool shown: loadedUI && wideAspect && player.currentMetaSource !== ""

        Behavior on width {
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
