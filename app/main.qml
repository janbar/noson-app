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
import Ubuntu.Components 1.3
import Ubuntu.Components.Popups 1.3
import Ubuntu.Content 1.1
//import Ubuntu.Connectivity 1.3
import Qt.labs.settings 1.0
import QtGraphicalEffects 1.0
import NosonApp 1.0
import "./components/BottomEdge"
import "./components/Dialog"
import "./components"
import "./ui"

/*!
    \brief MainView with Tabs element.
           First Tab has a single Label and
           second Tab has a single ToolbarAction.
*/

MainView {
    objectName: "nosonMainView"
    applicationName: "noson.janbar"
    id: mainView

    readonly property string versionString: "2.8.1"

    focus: true
    backgroundColor: styleMusic.mainView.backgroundColor

    Binding {
        target: theme
        property: "name"
        value: "Ubuntu.Components.Themes.SuruDark"
    }

    // This property enables the application to change orientation
    // when the device is rotated. The default is false.
    automaticOrientation: true

    // Design stuff
    Style { id: styleMusic }
    width: units.gu(142)
    height: units.gu(80)

    // Arguments during startup
    Arguments {
        id: args
        // Debug/development mode
        Argument {
            name: "debug"
            help: "Start Noson in a debug mode. Will show more output."
            required: false
        }
        Argument {
            name: "scalefactor"
            help: "set a scale factor"
            valueNames: ["FACTOR"]
            required: false
        }
    }

    // Startup settings
    Settings {
        id: startupSettings
        //catagory: "StartupSettings"

        property bool firstRun: true
        property string zoneName: ""
        property int tabIndex: -1
        property double width: mainView.width
        property double height: mainView.height
        property real scaleFactor: 1.0
        property string accounts: ""
    }

    // The player handles all actions to control the music
    Player {
        id: player
    }

    // Variables
    property string appName: i18n.tr("Noson")
    property int debugLevel: 1

    // Property to store the state of an application (active or suspended)
    property bool applicationState: Qt.application.active

    // setting alias to check first run
    property alias firstRun: startupSettings.firstRun

    // setting alias to store last zone connected
    property alias currentZone: startupSettings.zoneName
    property string currentZoneTag: ""

    // track latest stream link
    property string inputStreamUrl: ""

    // Device isn't connected to network: true/false
    // property bool noNetwork: !NetworkingStatus.online || NetworkingStatus.limitedBandwith

    // No source configured: First user has to select a source
    property bool noMusic: player.currentMetaSource === "" && loadedUI

    // No zone connected: UI push page "NoZoneState" on top to invit user to retry discovery of Sonos devices
    property bool noZone: false // doesn't pop page on startup
    property Page emptyPage

    // current page now playing
    property Page nowPlayingPage

    // property to detect if the UI has finished
    property bool loadedUI: false
    property bool wideAspect: width >= units.gu(100) && loadedUI

    // property to enable pop info on index loaded
    property bool infoLoadedIndex: true // enabled at startup

    // Constants
    readonly property int queueBatchSize: 100

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Events
    ////

    Connections {
            target: Sonos

            onJobCountChanged: jobRunning = Sonos.jobCount > 0 ? true : false

            onInitDone: {
                if (succeeded) {
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
                    popInfo.open(i18n.tr("Index loaded"));
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
                }
            }
        }
    }

    // Run on startup
    Component.onCompleted: {
        if (args.values.debug) {
            mainView.debugLevel = 4
        }
        if (args.values.scalefactor) {
            startupSettings.scaleFactor = args.values.scalefactor
        }
        units.gridUnit *= startupSettings.scaleFactor;
        customdebug("LANG=" + Qt.locale().name);
        Sonos.setLocale(Qt.locale().name);

        // init SMAPI third party accounts
        var acls = deserializeACLS(startupSettings.accounts);
        for (var i = 0; i < acls.length; ++i) {
            customdebug("register account: type=" + acls[i].type + " sn=" + acls[i].sn + " token=" + acls[i].token.substr(0, 1) + "...");
            Sonos.addServiceOAuth(acls[i].type, acls[i].sn, acls[i].key, acls[i].token, acls[i].username);
        }

        // initialize all data models
        AllZonesModel.init(Sonos, "",       false);
        AllFavoritesModel.init(Sonos, "",   false);
        AllServicesModel.init(Sonos,        false);
        AllAlbumsModel.init(Sonos, "",      false);
        AllArtistsModel.init(Sonos, "",     false);
        AllGenresModel.init(Sonos, "",      false);
        AllPlaylistsModel.init(Sonos, "",   false);
        MyServicesModel.init(Sonos,         false);

        // push the page to view
        mainPageStack.push(tabs)

        // launch connection
        connectSonos();

        // if a tab index exists restore it, otherwise goto Recent if there are items otherwise go to Albums
        tabs.selectedTabIndex = startupSettings.tabIndex === -1
                ? servicesTab.index
                : (startupSettings.tabIndex > tabs.count - 1
                   ? tabs.count - 1 : startupSettings.tabIndex)

        // signal UI has finished
        loadedUI = true;

        // resize main view according to user settings
        mainView.width = (startupSettings.width >= units.gu(44) ? startupSettings.width : units.gu(44));
        mainView.height = (startupSettings.height >= units.gu(80) ? startupSettings.height : units.gu(80));

        //@TODO add url to play list
        //if (args.values.url) {
        //}
    }

/*    Timer {
        id: delayStartup
        interval: 100
        onTriggered: {
            // push the page to view
            mainPageStack.push(tabs)

            // try to connect the controller
            connectSonos()

            // if a tab index exists restore it, otherwise goto Recent if there are items otherwise go to Albums
            tabs.selectedTabIndex = startupSettings.tabIndex === -1
                    ? servicesTab.index
                    : (startupSettings.tabIndex > tabs.count - 1
                       ? tabs.count - 1 : startupSettings.tabIndex)

            // signal UI has finished
            loadedUI = true;

            if (args.values.url) {
                //@TODO add url to play list
            }
        }
    }*/

    // Show/hide page NoZoneState
    onNoZoneChanged: {
        if (noZone) {
            emptyPage = mainPageStack.push(Qt.resolvedUrl("ui/NoZoneState.qml"), {})
        } else {
            mainPageStack.popPage(emptyPage)
        }
    }

    // Refresh player state when application becomes active
    onApplicationStateChanged: {
        if (!noZone && applicationState && player.connected) {
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
        target: AllPlaylistsModel
        onDataUpdated: AllPlaylistsModel.asyncLoad()
        onLoaded: AllPlaylistsModel.resetModel()
    }


    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global keyboard shortcuts
    ////

    Keys.onPressed: {
        if(event.key === Qt.Key_Escape) {
            if (mainPageStack.currentMusicPage.currentDialog !== null) {
                PopupUtils.close(mainPageStack.currentMusicPage.currentDialog)
            } else if (mainPageStack.currentMusicPage.searchable && mainPageStack.currentMusicPage.state === "search") {
                mainPageStack.currentMusicPage.state = "default"
            } else {
                mainPageStack.goBack();  // Esc: Go back
            }
        }
        else if(event.modifiers === Qt.AltModifier) {
            var position;

            switch (event.key) {
            case Qt.Key_Right:  //  Alt+Right   Seek forward +10secs
                position = player.position + 10000 < player.duration
                        ? player.position + 10000 : player.duration;
                player.seek(position);
                break;
            case Qt.Key_Left:  //   Alt+Left    Seek backwards -10secs
                position = player.position - 10000 > 0
                        ? player.position - 10000 : 0;
                player.seek(position);
                break;
            }
        }
        else if(event.modifiers === Qt.ControlModifier) {
            switch (event.key) {
            case Qt.Key_Left:   //  Ctrl+Left   Previous Song
                player.previousSong(true);
                break;
            case Qt.Key_Right:  //  Ctrl+Right  Next Song
                player.nextSong(true, true);
                break;
            case Qt.Key_Up:  //     Ctrl+Up     Volume up
            {
                var v = player.volumeMaster + 5 > 100 ? 100 : player.volumeMaster + 5;
                if (player.setVolumeGroup(v))
                    player.volumeMaster = Math.round(v);
                break;
            }
            case Qt.Key_Down:  //   Ctrl+Down   Volume down
            {
                var v = player.volumeMaster - 5 < 0 ? 0 : player.volumeMaster - 5;
                if (player.setVolumeGroup(v))
                    player.volumeMaster = Math.round(v);
                break;
            }
            case Qt.Key_R:  //      Ctrl+R      Repeat toggle
            {
                var old = player.repeat
                if (player.toggleRepeat())
                    player.repeat = !old
                break;
            }
            case Qt.Key_F:  //      Ctrl+F      Show Search popup
                if (mainPageStack.currentMusicPage.searchable && mainPageStack.currentMusicPage.state === "default") {
                    mainPageStack.currentMusicPage.state = "search"
                    header.show()
                }

                break;
            case Qt.Key_J:  //      Ctrl+J      Jump to playing song
                tabs.pushNowPlaying()
                mainPageStack.currentPage.setListView(true)
                break;
            case Qt.Key_N:  //      Ctrl+N      Show Now playing
                tabs.pushNowPlaying()
                break;
            case Qt.Key_P:  //      Ctrl+P      Toggle playing state
                player.toggle();
                break;
            case Qt.Key_Q:  //      Ctrl+Q      Quit the app
                Qt.quit();
                break;
            case Qt.Key_U:  //      Ctrl+U      Shuffle toggle
            {
                var old = player.shuffle
                if (player.toggleShuffle())
                    player.shuffle = !old
                break;
            }
            default:
                break;
            }
        }
        else {
            switch (event.key) {
            case Qt.Key_MediaPlay:  //   Play   Toggle playing state
            case Qt.Key_MediaPause:  //  Pause  Toggle playing state
                player.toggle();
                break;
            case Qt.Key_MediaPrev:   //  Prev   Previous Song
                player.previousSong(true);
                break;
            case Qt.Key_MediaNext:  //   Next   Next Song
                player.nextSong(true, true);
                break;
            }
        }
    }

    Connections {
        target: ContentHub
        onShareRequested: {
            var url = transfer.items[0].url
            if (!player.startPlayStream(url, ""))
                popInfo.open(i18n.tr("Action can't be performed"))
            else
                inputStreamUrl = url
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global actions & helpers
    ////

    // Custom debug funtion that's easier to shut off
    function customdebug(text) {
        var debug = true; // set to "0" for not debugging
        //if (args.values.debug) { // *USE LATER*
        if (debug) {
            console.debug(i18n.tr("Debug: ")+text);
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
            if (attrs.length == 3) // <= 2.4.7
                acls.push({type: attrs[0], sn: attrs[1], key: attrs[2], token: attrs[2], username: ""});
            else if (attrs.length == 4) // >= 2.4.8
                acls.push({type: attrs[0], sn: attrs[1], key: Qt.atob(attrs[2]), token: Qt.atob(attrs[3]), username: ""});
            else if (attrs.length == 5)
                acls.push({type: attrs[0], sn: attrs[1], key: Qt.atob(attrs[2]), token: Qt.atob(attrs[3]), username: Qt.atob(attrs[4])});
        }
        return acls;
    }

    // Try connect to SONOS system
    function connectSonos() {
        return Sonos.startInit(mainView.debugLevel);
    }

    signal zoneChanged

    // Try to change zone
    // On success noZone is set to false
    function connectZone(name) {
        var oldZone = currentZone;
        customdebug("Connecting zone '" + name + "'");
        if ((Sonos.connectZone(name) || Sonos.connectZone("")) && player.connect()) {
            currentZone = Sonos.getZoneName();
            currentZoneTag = Sonos.getZoneShortName();
            if (currentZone !== oldZone)
                zoneChanged();
            if (noZone)
                noZone = false;
            return true;
        } else {
            if (!noZone)
                noZone = true;
        }
        return false;
    }

    // Action on request to update music library
    function updateMusicIndex() {
        if (player.refreshShareIndex()) {
            // enable info on loaded index
            infoLoadedIndex = true;
            popInfo.open(i18n.tr("Refreshing of index is running"));
            return true;
        }
        popInfo.open(i18n.tr("Action can't be performed"));
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
            popInfo.open(i18n.tr("Action can't be performed"));
            return false;
        }
        else {
            if (player.addItemToQueue(modelItem, nr)) {
                popInfo.open(i18n.tr("song added"));
                return true;
            }
            popInfo.open(i18n.tr("Action can't be performed"));
            return false;
        }
    }

    // Action on track from queue clicked
    function indexQueueClicked(index) {
        if (player.currentIndex === index) {
            if (player.toggle())
                return true;
            popInfo.open(i18n.tr("Action can't be performed"));
            return false;
        }
        else if (player.playQueue(false) && player.seekTrack(index + 1) && player.play())
            return true;
        popInfo.open(i18n.tr("Action can't be performed"));
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
            popInfo.open(i18n.tr("%1 song added", "%1 songs added", modelItemList.length).arg(modelItemList.length));
            return true;
        }
        popInfo.open(i18n.tr("Action can't be performed"));
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
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    // Action add queue
    function addQueue(modelItem)
    {
        var nr = player.trackQueue.model.count;
        if (player.addItemToQueue(modelItem, ++nr) > 0) {
            popInfo.open(i18n.tr("song added"));
            return true;
        }
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    // Action delete all tracks from queue
    function removeAllTracksFromQueue()
    {
        if (player.removeAllTracksFromQueue()) {
            popInfo.open(i18n.tr("Queue cleared"));
            return true;
        }
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    // Action on remove queue track
    function removeTrackFromQueue(modelItem) {
        if (player.removeTrackFromQueue(modelItem))
            return true;
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    // Action on move queue item
    function reorderTrackInQueue(from, to) {
        if (from < to)
            ++to;
        if (player.reorderTrackInQueue(from + 1, to + 1))
            return true;
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    // Action on radio item clicked
    function radioClicked(modelItem) {
        if (player.setSource(modelItem) && player.play()) {
            tabs.pushNowPlaying();
            return true;
        }
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    function saveQueue(title) {
        if (player.saveQueue(title))
            return true;
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    function createPlaylist(title) {
        if (player.createSavedQueue(title))
            return true;
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    function addPlaylist(playlistId, modelItem, containerUpdateID) {
        if (player.addItemToSavedQueue(playlistId, modelItem, containerUpdateID)) {
            popInfo.open(i18n.tr("song added"));
            return true;
        }
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    function removeTracksFromPlaylist(playlistId, selectedIndices, containerUpdateID) {
        if (player.removeTracksFromSavedQueue(playlistId, selectedIndices, containerUpdateID)) {
            popInfo.open(i18n.tr("%1 song removed", "%1 songs removed", selectedIndices.length).arg(selectedIndices.length));
            return true;
        }
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    // Action on move playlist item
    function reorderTrackInPlaylist(playlistId, from, to, containerUpdateID) {
        if (player.reorderTrackInSavedQueue(playlistId, from, to, containerUpdateID))
            return true;
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    function removePlaylist(itemId) {
        if (player.destroySavedQueue(itemId))
            return true;
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    function addItemToFavorites(modelItem, description, artURI) {
        if (player.addItemToFavorites(modelItem, description, artURI))
            return true;
        popInfo.open(i18n.tr("Action can't be performed"));
        return false;
    }

    function removeFromFavorites(itemPayload) {
        var id = AllFavoritesModel.findFavorite(itemPayload)
        if (id.length === 0) // no favorite
            return true;
        if (player.removeFavorite(id))
            return true;
        popInfo.open(i18n.tr("Action can't be performed"));
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

    function makeCoverSource(art, artist, album) {
        var uri = undefined;
        if (art !== undefined && art !== "")
            uri = art;
        else if (album !== undefined && album !== "")
            uri = Qt.resolvedUrl("graphics/no_cover.png");
        else if (artist !== undefined && artist !== "")
            uri = Qt.resolvedUrl("graphics/none.png");
        return uri;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Display items
    ////

    PopInfo {
        id: popInfo
    }

    DialogSongInfo {
        id: dialogSongInfo
    }

    Loader {
        id: zonesPageLoader
        asynchronous: false
        source: "ui/Zones.qml"
        visible: false
    }

    PageStack {
        id: mainPageStack
        anchors {
            bottom: parent.bottom
            fill: undefined
            left: parent.left
            right: nowPlayingSidebarLoader.left
            top: parent.top
        }
        clip: true  // otherwise listitems actions overflow

        // Properties storing the current page info
        property Page currentMusicPage: null  // currentPage can be Tabs
        property bool popping: false

        /* Helper functions */

        // Go back up the stack if possible
        function goBack() {
            // Ensure in the case that goBack is called programmatically that any dialogs are closed
            if (mainPageStack.currentMusicPage !== null && mainPageStack.currentMusicPage.currentDialog !== null) {
                PopupUtils.close(mainPageStack.currentMusicPage.currentDialog)
            }
            if (depth > 1) {
                // Check popping of now playing
                if (mainPageStack.currentPage === nowPlayingPage) {
                    nowPlayingPage = null
                }
                pop()
            }
        }

        // Pop a specific page in the stack
        function popPage(page) {
            var tmpPages = []

            // Ensure in the case that popPage is called programmatically that any dialogs are closed
            if (page && page.currentDialog !== undefined && page.currentDialog !== null) {
                PopupUtils.close(page.currentDialog)
            }

            popping = true

            while (currentPage !== page && depth > 0) {
                tmpPages.push(currentPage)
                pop()
            }

            if (depth > 0) {
                pop()
            }

            for (var i=tmpPages.length - 1; i > -1; --i) {
                push(tmpPages[i])
            }

            popping = false
        }

        // Set the current page, and any parent/stacks
        function setPage(childPage) {
            if (!popping) {
                currentMusicPage = childPage;
            }
        }

        Tabs {
            id: tabs
            anchors {
                fill: parent
            }

            property Tab lastTab: selectedTab
            property list<Action> tabActions: [
                Action {
                    objectName: "servicesTabAction"
                    text: servicesTab.title
                    onTriggered: tabs.selectedTabIndex = servicesTab.index
                    visible: AllServicesModel.count > 0 ? true : false
                },
                Action {
                    objectName: "artistsTabAction"
                    text: artistsTab.title
                    onTriggered: tabs.selectedTabIndex = artistsTab.index
                    visible: AllArtistsModel.count > 0 ? true : false
                },
                Action {
                    objectName: "albumsTabAction"
                    text: albumsTab.title
                    onTriggered: tabs.selectedTabIndex = albumsTab.index
                    visible: AllAlbumsModel.count > 0 ? true : false
                },
                Action {
                    objectName: "genresTabAction"
                    text: genresTab.title
                    onTriggered: tabs.selectedTabIndex = genresTab.index
                    visible: AllGenresModel.count > 0 ? true : false
                },
                Action {
                    objectName: "playlistsTabAction"
                    text: playlistsTab.title
                    onTriggered: tabs.selectedTabIndex = playlistsTab.index
                    visible: true
                },
                Action {
                    objectName: "favoritesTabAction"
                    text: favoritesTab.title
                    onTriggered: tabs.selectedTabIndex = favoritesTab.index
                    visible: AllFavoritesModel.count > 0 ? true : false
                }
            ]

            onSelectedTabChanged: {
                lastTab = selectedTab;
            }

            onSelectedTabIndexChanged: {
                if (loadedUI) {  // store the tab index if changed by the user
                    startupSettings.tabIndex = selectedTabIndex
                }
            }

            Tab {
                id: servicesTab
                objectName: "servicesTab"
                anchors.fill: parent
                title: page.pageTitle

                // tab content
                page: MusicServices {
                    id: servicesPage
                }
            }

            Tab {
                id: artistsTab
                objectName: "artistsTab"
                anchors.fill: parent
                title: page.pageTitle

                // tab content
                page: Artists {
                    id: artistsPage
                }
            }

            Tab {
                id: albumsTab
                objectName: "albumsTab"
                anchors.fill: parent
                title: page.pageTitle

                // Tab content begins here
                page: Albums {
                    id: albumsPage
                }
            }

            Tab {
                id: genresTab
                objectName: "genresTab"
                anchors.fill: parent
                title: page.pageTitle

                // Tab content begins here
                page: Genres {
                    id: genresPage
                }
            }

            Tab {
                id: playlistsTab
                objectName: "playlistsTab"
                anchors.fill: parent
                title: page.pageTitle

                // Tab content begins here
                page: Playlists {
                    id: playlistsPage
                }
            }

            Tab {
                id: favoritesTab
                objectName: "favoritesTab"
                anchors.fill: parent
                title: page.pageTitle

                // Tab content begins here
                page: Favorites {
                    id: favoritesPage
                }
            }

            /*Tab {
                id: songsTab
                objectName: "songsTab"
                anchors.fill: parent
                title: page.pageTitle

                // Tab content begins here
                page: Songs {
                    id: tracksPage
                }
            }*/

            function pushNowPlaying()
            {
                if (!wideAspect) {
                    // pop existing page now playing
                    if (nowPlayingPage !== null) {
                        mainPageStack.popPage(nowPlayingPage);
                        nowPlayingPage = null;
                    }
                    nowPlayingPage = mainPageStack.push(Qt.resolvedUrl("ui/NowPlaying.qml"), {});
                    if (nowPlayingPage.isListView === true) {
                        nowPlayingPage.setListView(false);  // ensure full view
                    }
                }
            }
        } // end of tabs
    }

    //
    // Components that are ontop of the PageStack
    //

    Loader {
        id: nowPlayingSidebarLoader
        active: shown
        anchors {  // start offscreen
            bottom: parent.bottom
            right: parent.right
            top: parent.top
        }
        asynchronous: true
        source: "components/NowPlayingSidebar.qml"
        width: shown && status === Loader.Ready ? units.gu(44) : 0
        visible: width > 0

        property bool shown: loadedUI && wideAspect && player.currentMetaSource !== ""

        Behavior on width {
            NumberAnimation {

            }
        }
    }

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
        source: "components/MusicToolbar.qml"
        visible: !noZone && (!wideAspect || player.currentMetaSource === "") && status === Loader.Ready &&
                 (mainPageStack.currentPage && (mainPageStack.currentPage.showToolbar || mainPageStack.currentPage.showToolbar === undefined))

        height: status === Loader.Ready ? item.height : 0
    }

    property bool jobRunning: false

    LoadingSpinnerComponent {
        id: loading
        visible: jobRunning
    }
}
