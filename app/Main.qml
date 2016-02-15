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
//import Ubuntu.Connectivity 1.3
import Qt.labs.settings 1.0
import QtMultimedia 5.0
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
    width: units.gu(46)
    height: units.gu(80)

    // Arguments during startup
    Arguments {
        id: args
        //defaultArgument.help: "Expects URI of the track to play."
        //defaultArgument.valueNames: ["URI"]
        // grab a media
        Argument {
            name: "url"
            help: "URI for track to run at start."
            required: false
            valueNames: ["track"]
        }
        // Debug/development mode
        Argument {
            name: "debug"
            help: "Start Noson in a debug mode. Will show more output."
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

    // Device isn't connected to network: true/false
    // property bool noNetwork: !NetworkingStatus.online || NetworkingStatus.limitedBandwith

    // No source configured: First user has to select a source
    property bool noMusic: player.currentMetaSource === "" && loadedUI

    // No zone connected: UI push page "NoZoneState" on top to invit user to retry discovery of Sonos devices
    property bool noZone: false // doesn't pop page on startup
    property Page emptyPage

    // property to detect if the UI has finished
    property bool loadedUI: false
    property bool wideAspect: width >= units.gu(70) && loadedUI

    // property to enable pop info on index loaded
    property bool infoLoadedIndex: false

    // Constants
    readonly property int queueBatchSize: 100

    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Events
    ////

    // Run on startup
    Component.onCompleted: {

        // push the page to view
        mainPageStack.push(tabs)

        currentlyWorking = true

        // try to connect a zone
        // currentZone is alias of startupSettings.zoneName
        connectZone(currentZone)

        // if a tab index exists restore it, otherwise goto Recent if there are items otherwise go to Albums
        tabs.selectedTabIndex = startupSettings.tabIndex === -1
                ? albumsTab.index
                : (startupSettings.tabIndex > tabs.count - 1
                   ? tabs.count - 1 : startupSettings.tabIndex)

        // signal UI has finished
        loadedUI = true;

        if (args.values.url) {
            //@TODO add url to play list
        }

    }

    // Show/hide page NoZoneState
    onNoZoneChanged: {
        if (noZone) {
            currentlyWorking = false // hide actvity indicator
            emptyPage = mainPageStack.push(Qt.resolvedUrl("ui/NoZoneState.qml"), {})
        } else {
            mainPageStack.popPage(emptyPage)
        }
    }

    // Refresh player state when application becomes active
    onApplicationStateChanged: {
        if (!noZone && applicationState && player.connected) {
            mainView.currentlyWorking = true
            delayPlayerWakeUp.start()
        }
    }

    Timer {
        id: delayPlayerWakeUp
        interval: 100
        onTriggered: {
            noZone = !player.wakeUp()
            mainView.currentlyWorking = false
        }
    }

    // About backend signals
    // Triggers asynchronous loading on dataUpdated from global models
    // For these singletons, data loading is processed by backend threads.
    // Invoking asyncLoad() will schedule the reloading of data.
    Connections {
        target: AllZonesModel
        onDataUpdated: AllZonesModel.asyncLoad()
    }

    Connections {
        target: AllAlbumsModel
        onDataUpdated: AllAlbumsModel.asyncLoad()
    }

    Connections {
        target: AllArtistsModel
        onDataUpdated: AllArtistsModel.asyncLoad()
    }

    Connections {
        target: AllGenresModel
        onDataUpdated: AllGenresModel.asyncLoad()
    }

    Connections {
        target: AllRadiosModel
        onDataUpdated: AllRadiosModel.asyncLoad()
    }

    Connections {
        target: AllPlaylistsModel
        onDataUpdated: AllPlaylistsModel.asyncLoad()
    }

    Connections {
        target: Sonos
        onLoadingFinished: {
            if (infoLoadedIndex) {
                infoLoadedIndex = false;
                popInfo.open(i18n.tr("Index loaded"));
            }
            currentlyWorking = false; // hide actvity indicator
        }
    }

    // Global keyboard shortcuts
    focus: true
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
                mainPageStack.currentPage.isListView = true
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

    // Try to connect zone
    // On success: noZone is set to false and content loader thread is started
    // to fill data in global models
    // On failure: noZone is set to true
    function connectZone(name) {
        // by default try currentZone if any
        name = (name === undefined || name === "") ? currentZone : name;

        if (Sonos.init(debugLevel)) {
            customdebug("Connecting zone '" + name + "'");
            if ((Sonos.connectZone(name) || Sonos.connectZone("")) && player.connect()) {
                currentZone = Sonos.getZoneName();
                AllZonesModel.init(Sonos, true); // force load now
                AllAlbumsModel.init(Sonos, "");
                AllArtistsModel.init(Sonos, "");
                AllGenresModel.init(Sonos, "");
                AllRadiosModel.init(Sonos, "R:0/0");
                AllPlaylistsModel.init(Sonos, "");
                // enable info on index loaded
                infoLoadedIndex = true;
                Sonos.runLoader();
                // Signal change if any
                if (noZone)
                    noZone = false;
                return true;
            }
        }
        // Signal change if any
        if (!noZone)
            noZone = true;
        return false;
    }

    // Reload zones and try connect
    function reloadZone() {
        if (Sonos.init(debugLevel)) {
            customdebug("Connecting zone '" + currentZone + "'");
            if ((Sonos.connectZone(currentZone) || Sonos.connectZone("")) && player.connect()) {
                currentZone = Sonos.getZoneName();
                AllZonesModel.init(Sonos, true); // force load now
                // Signal change if any
                if (noZone)
                    noZone = false;
                return true;
            }
        }
        // Signal change if any
        if (!noZone)
            noZone = true;
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
        var index = Math.floor(model.count * Math.random(seed));
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
        var nr = player.addItemToQueue(modelItem, 0);
        if (nr && player.playQueue(false) && player.seekTrack(nr) && player.play()) {
            // Show the Now playing page and make sure the track is visible
            tabs.pushNowPlaying();
            popInfo.open(i18n.tr("song added"));
            return true;
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
        id: musicToolbar
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        asynchronous: true
        source: "components/MusicToolbar.qml"
        visible: (mainPageStack.currentPage.showToolbar || mainPageStack.currentPage.showToolbar === undefined) &&
                 !noZone

        z: 200  // put on top of everything else
    }

    Loader {
        id: zonesPageLoader
        asynchronous: false
        source: "ui/Zones.qml"
        visible: false
    }

    PageStack {
        id: mainPageStack

        // Properties storing the current page info
        property Page currentMusicPage: null  // currentPage can be Tabs
        property bool popping: false

        // Property to detect if page 'Now playing' is in the flow and then disable navigation from the toolbar
        property bool nowPlayingLoaded: false

        /* Helper functions */

        // Go back up the stack if possible
        function goBack() {
            // Ensure in the case that goBack is called programmatically that any dialogs are closed
            if (mainPageStack.currentMusicPage.currentDialog !== null) {
                PopupUtils.close(mainPageStack.currentMusicPage.currentDialog)
            }
            if (depth > 1) {
                // Check popping of now playing
                if (mainPageStack.currentPage.title === i18n.tr("Now playing"))
                    mainPageStack.nowPlayingLoaded = false
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

            onSelectedTabChanged: {
                lastTab = selectedTab;
            }

            onSelectedTabIndexChanged: {
                if (loadedUI) {  // store the tab index if changed by the user
                    startupSettings.tabIndex = selectedTabIndex
                }
            }

            Tab {
                id: artistsTab
                objectName: "artistsTab"
                anchors.fill: parent
                title: page.title

                // tab content
                page: Artists {
                    id: artistsPage
                }
            }

            Tab {
                id: albumsTab
                objectName: "albumsTab"
                anchors.fill: parent
                title: page.title

                // Tab content begins here
                page: Albums {
                    id: albumsPage
                }
            }

            Tab {
                id: genresTab
                objectName: "genresTab"
                anchors.fill: parent
                title: page.title

                // Tab content begins here
                page: Genres {
                    id: genresPage
                }
            }

            Tab {
                id: radiosTab
                objectName: "radiosTab"
                anchors.fill: parent
                title: page.title

                // Tab content begins here
                page: Radios {
                    id: radiosPage
                }
            }

            Tab {
                id: playlistsTab
                objectName: "playlistsTab"
                anchors.fill: parent
                title: page.title

                // Tab content begins here
                page: Playlists {
                    id: playlistsPage
                }
            }

            /*Tab {
                id: songsTab
                objectName: "songsTab"
                anchors.fill: parent
                title: page.title

                // Tab content begins here
                page: Songs {
                    id: tracksPage
                }
            }*/

            function pushNowPlaying()
            {
                // only push if not already loaded
                if (!mainPageStack.nowPlayingLoaded) {
                    mainPageStack.nowPlayingLoaded = true
                    mainPageStack.push(Qt.resolvedUrl("ui/NowPlaying.qml"), {})
                }

                if (mainPageStack.currentPage.isListView === true) {
                    mainPageStack.currentPage.isListView = false;  // ensure full view
                }
            }
        } // end of tabs
    }

    property alias currentlyWorking: loading.visible

    LoadingSpinnerComponent {
        id: loading
    }
}
