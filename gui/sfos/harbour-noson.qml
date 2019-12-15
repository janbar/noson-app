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

    initialPage: Component {
        MusicServices {
        }
    }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    ConfigurationGroup {
        id: settings
        path: "/uk/co/piggz/noson"
        synchronous: true
        
        property real scaleFactor: 1.0
        property real fontScaleFactor: 1.0
        property bool firstRun: true
        property string zoneName
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
        scaleFactor: 4.0
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

    // Property to store the state of an application (active or suspended)
    property bool applicationSuspended: Qt.application.state === Qt.ApplicationSuspended

    // setting alias to check first run
    //property alias firstRun: settings.firstRun

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
    //property real wideSongView: units.gu(70)

    // property to enable pop info on index loaded
    property bool infoLoadedIndex: true // enabled at startup

    // property to detect thumbnailer is available
    property bool thumbValid: false

    // Constants
    readonly property int queueBatchSize: 100
    readonly property real minSizeGU: 42
    readonly property string tr_undefined: qsTr("<Undefined>")

    // Cache built-in genre artworks
    property var genreArtworks: []
    
    property bool alarmEnabled: false
    property bool shareIndexInProgress: false
    
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
            AllZonesModel.asyncLoad()
            delayReloadZone.start()
        }
    }

    Timer {
        id: delayReloadZone
        interval: 250
        onTriggered: {
            if (jobRunning) {
                restart();
            } else {
                // Reload the zone and start the content loader thread
                customdebug("Reloading the zone ...");
                if (connectZone(currentZone)) {
                    Sonos.runLoader();
                    // Completing startup
                    if (startup) {
                        startup = false;
                        if (playOnStart) {
                            if (player.playStream(inputStreamUrl, "")) {
                                tabs.pushNowPlaying();
                            }
                        }
                        // check for enabled alarm
                        alarmEnabled = isAlarmEnabled();
                    }
                }
            }
        }
    }

    // Run on startup
    Component.onCompleted: {
        var argno = 0;

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
            if (pageStack.currentItem === noZonePage) {
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
        // Some tasks are already launched during startup
        if (!startup) {
          alarmEnabled = isAlarmEnabled();
        }
    }

    
    
    
    ////
    //// Global actions & helpers
    ////

    // Find index of a command line argument else -1
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
            customdebug("NOTICE: Connecting using the configured URL: " + deviceUrl)
            ssdp = false // point out the ssdp discovery isn't used to connect
            if (Sonos.init(debugLevel, deviceUrl))
                return true
            customdebug("ERROR: Connection has failed using the configured URL: " + deviceUrl)
        }
        ssdp = true // point out the ssdp discovery is used to connect
        return Sonos.startInit(debugLevel)
    }

    signal zoneChanged

    // Try to change zone
    // On success noZone is set to false
    function connectZone(name) {
        var oldZone = currentZone
        customdebug("Connecting zone '" + name + "'")
        if ((Sonos.connectZone(name) || Sonos.connectZone(""))
                && player.connect()) {
            currentZone = Sonos.getZoneName()
            currentZoneTag = Sonos.getZoneShortName()
            if (currentZone !== oldZone)
                zoneChanged()
            if (noZone)
                noZone = false
            return true
        } else {
            if (!noZone)
                noZone = true
        }
        return false
    }

    // Action on request to update music library
    function updateMusicIndex() {
        if (player.refreshShareIndex()) {
            // enable info on loaded index
            infoLoadedIndex = true
            popInfo.open(qsTr("Refreshing of index is running"))
            return true
        }
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    // Action on track clicked
    function trackClicked(modelItem, play) {
        play = play === undefined ? true : play // default play to true
        var nr = player.trackQueue.model.count + 1
        // push back
        if (play) {
            if (player.playQueue(false)) {
                nr = player.addItemToQueue(modelItem, nr)
                if (player.seekTrack(nr) && player.play()) {
                    // Show the Now playing page and make sure the track is visible
                    tabs.pushNowPlaying()
                    return true
                }
            }
            popInfo.open(qsTr("Action can't be performed"))
            return false
        } else {
            if (player.addItemToQueue(modelItem, nr)) {
                popInfo.open(qsTr("song added"))
                return true
            }
            popInfo.open(qsTr("Action can't be performed"))
            return false
        }
    }

    // Action on track from queue clicked
    function indexQueueClicked(index) {
        if (player.currentIndex === index) {
            if (player.toggle())
                return true
            popInfo.open(qsTr("Action can't be performed"))
            return false
        } else if (player.playQueue(false) && player.seekTrack(index + 1)
                   && player.play())
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
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
        if (player.addMultipleItemsToQueue(modelItemList)) {
            popInfo.open(qsTr("%n song(s) added", "", modelItemList.length))
            return true
        }
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    // Action on play all button clicked
    function playAll(modelItem) {
        // replace queue with the bundle item
        if (player.removeAllTracksFromQueue()) {
            var nr = player.addItemToQueue(modelItem, 0)
            if (nr && player.playQueue(false) && player.seekTrack(nr)
                    && player.play()) {
                // Show the Now playing page and make sure the track is visible
                tabs.pushNowPlaying()
                popInfo.open(qsTr("song added"))
                return true
            }
        }
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    // Action add queue
    function addQueue(modelItem) {
        var nr = player.trackQueue.model.count
        if (player.addItemToQueue(modelItem, ++nr) > 0) {
            popInfo.open(qsTr("song added"))
            return true
        }
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    // Action delete all tracks from queue
    function removeAllTracksFromQueue() {
        if (player.removeAllTracksFromQueue()) {
            popInfo.open(qsTr("Queue cleared"))
            return true
        }
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    // Action on remove queue track
    function removeTrackFromQueue(modelItem) {
        if (player.removeTrackFromQueue(modelItem))
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
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

    function saveQueue(title) {
        if (player.saveQueue(title))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    function createPlaylist(title) {
        if (player.createSavedQueue(title))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    function addPlaylist(playlistId, modelItem, containerUpdateID) {
        if (player.addItemToSavedQueue(playlistId, modelItem, containerUpdateID)) {
            popInfo.open(qsTr("song added"));
            return true;
        }
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

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
    function removePlaylist(itemId) {
        if (player.destroySavedQueue(itemId))
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    function addItemToFavorites(modelItem, description, artURI) {
        if (player.addItemToFavorites(modelItem, description, artURI))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    function removeFromFavorites(itemPayload) {
        var id = AllFavoritesModel.findFavorite(itemPayload)
        if (id.length === 0)
            // no favorite
            return true
        if (player.removeFavorite(id))
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
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
        var rooms = Sonos.getZoneRooms()
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
            if (nowPlayingPage === null)
                nowPlayingPage = pageStack.push("qrc:/sfos/pages/NowPlaying.qml", false, true);
            if (nowPlayingPage.isListView) {
                nowPlayingPage.isListView = false; // ensure full view
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

    DialogSleepTimer {
        id: dialogSleepTimer
    }

    DialogSongInfo {
        id: dialogSongInfo
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

    /*
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
        source: "qrc:/sfos/components/MusicToolbar.qml"
        //visible: !noZone && (player.currentMetaSource === "") && status === Loader.Ready &&
        //         (pageStack.currentItem && (pageStack.currentItem.showToolbar || pageStack.currentItem.showToolbar === undefined))
        visible: !noZone && (pageStack.currentPage && (pageStack.currentPage.showToolbar || pageStack.currentPage.showToolbar === undefined))

    }
    */
    
    //==============================================================
    // Spinner

    property bool jobRunning: false

    BusyIndicator {
        id: spinner
        anchors.centerIn: parent
        z: 100
        visible: jobRunning
        running: visible
    }
}
