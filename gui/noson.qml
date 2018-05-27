import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Controls.Universal 2.2
import Qt.labs.settings 1.0
import QtGraphicalEffects 1.0
import NosonApp 1.0
import "components"
import "components/Dialog"
import "ui"

ApplicationWindow {
    id: mainView
    visible: true
    title: "noson"

    readonly property string versionString: "3.3.8"

    // Design stuff
    width: 360
    height: 640

    Settings {
        id: settings
        property string style: "Default"
        property real scaleFactor: 1.0
        property real fontScaleFactor: 1.0
        property bool firstRun: true
        property string zoneName: ""
        property int tabIndex: -1
        property int widthGU: Math.round(mainView.width / units.gridUnit)
        property int heightGU: Math.round(mainView.height / units.gridUnit)
        property string accounts: ""
    }

    StyleLight {
        id: styleMusic
    }

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

    // Variables
    property string appName: "Noson"
    property int debugLevel: 1

    // Property to store the state of an application (active or suspended)
    property bool applicationState: Qt.application.active

    // setting alias to check first run
    property alias firstRun: settings.firstRun

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

    // Constants
    readonly property int queueBatchSize: 100
    readonly property real minSizeGU: 45

    minimumHeight: units.gu(minSizeGU)
    minimumWidth: units.gu(minSizeGU)

    // Cache built-in genre artworks
    property var genreArtworks: []

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
                }
            }
        }
    }

    // Run on startup
    Component.onCompleted: {
        if (indexOfArgument("--debug") >= 0) {
            mainView.debugLevel = 4
        }
        customdebug("LANG=" + Qt.locale().name);
        Sonos.setLocale(Qt.locale().name);

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
        AllAlbumsModel.init(Sonos, "",      false);
        AllArtistsModel.init(Sonos, "",     false);
        AllGenresModel.init(Sonos, "",      false);
        AllPlaylistsModel.init(Sonos, "",   false);
        MyServicesModel.init(Sonos,         false);
        // launch connection
        connectSonos();

        // signal UI has finished
        loadedUI = true;

        // resize main view according to user settings
        mainView.width = (settings.widthGU >= minSizeGU ? units.gu(settings.widthGU) : units.gu(minSizeGU));
        mainView.height = (settings.heightGU >= minSizeGU ? units.gu(settings.heightGU) : units.gu(minSizeGU));

        //@TODO add url to play list
        //if (args.values.url) {
        //}
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
        onCountChanged: { tabs.setProperty(1, "visible", (AllFavoritesModel.count > 0)) }
    }

    Connections {
        target: AllArtistsModel
        onDataUpdated: AllArtistsModel.asyncLoad()
        onLoaded: AllArtistsModel.resetModel()
        onCountChanged: { tabs.setProperty(2, "visible", (AllArtistsModel.count > 0)) }
    }

    Connections {
        target: AllAlbumsModel
        onDataUpdated: AllAlbumsModel.asyncLoad()
        onLoaded: AllAlbumsModel.resetModel()
        onCountChanged: { tabs.setProperty(3, "visible", (AllAlbumsModel.count > 0)) }
    }

    Connections {
        target: AllGenresModel
        onDataUpdated: AllGenresModel.asyncLoad()
        onLoaded: AllGenresModel.resetModel()
        onCountChanged: { tabs.setProperty(4, "visible", (AllGenresModel.count > 0)) }
    }

    Connections {
        target: AllPlaylistsModel
        onDataUpdated: AllPlaylistsModel.asyncLoad()
        onLoaded: AllPlaylistsModel.resetModel()
    }


    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global actions & helpers
    ////

    // Find index of a command line argument else -1
    function indexOfArgument(argv) {
        for (var i = 0; i < ApplicationArguments.length; ++i) {
            if (ApplicationArguments[i] === argv)
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
            if (attrs.length === 3) // <= 2.4.7
                acls.push({type: attrs[0], sn: attrs[1], key: attrs[2], token: attrs[2], username: ""});
            else if (attrs.length === 4) // >= 2.4.8
                acls.push({type: attrs[0], sn: attrs[1], key: Qt.atob(attrs[2]), token: Qt.atob(attrs[3]), username: ""});
            else if (attrs.length === 5)
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
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    function addItemToFavorites(modelItem, description, artURI) {
        if (player.addItemToFavorites(modelItem, description, artURI))
            return true;
        popInfo.open(qsTr("Action can't be performed"));
        return false;
    }

    function removeFromFavorites(itemPayload) {
        var id = AllFavoritesModel.findFavorite(itemPayload)
        if (id.length === 0) // no favorite
            return true;
        if (player.removeFavorite(id))
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

    function makeCoverSource(art, artist, album) {
        var uri = "qrc:/images/no_cover.png";
        if (art !== undefined && art !== "")
            uri = art;
        else if (album !== undefined && album !== "")
            uri = "qrc:/images/no_cover.png";
        else if (artist !== undefined && artist !== "")
            uri = "qrc:/images/none.png";
        return uri;
    }


    ////////////////////////////////////////////////////////////////////////////
    ////
    //// Global keyboard shortcuts
    ////

    Shortcut {
        sequences: ["Esc", "Back"]
        enabled: noZone === false && (mainToolBar.state === "search" || stackView.depth > 1)
        onActivated: {
            if (stackView.depth > 1) {
                if (stackView.currentItem.isRoot)
                    stackView.pop()
                else
                    stackView.currentItem.goUpClicked()
            } else if (mainToolBar.state === "search") {
                mainToolBar.state = "default"
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
            var position = player.position + 10000 < player.duration
                ? player.position + 10000 : player.duration;
            player.seek(position);
        }
    }
    Shortcut {
        sequence: "Alt+Left"            // Alt+Left    Seek backwards -10secs
        onActivated: {
            var position = player.position - 10000 > 0
                    ? player.position - 10000 : 0;
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
            if (stackView.currentItem.searchable && stackView.currentItem.state === "default") {
                stackView.currentItem.searchClicked()
            }
        }
    }
    Shortcut {
        sequence: "Ctrl+J"              // Ctrl+J      Jump to playing song
        onActivated: {
            tabs.pushNowPlaying()
            stackView.currentItem.isListView = true;
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

    property alias query: searchField.text

    header: ToolBar {
        id: mainToolBar
        Material.foreground: styleMusic.toolbar.foregroundColor
        Material.background: styleMusic.toolbar.fullBackgroundColor

        state: "default"
        states: [
            State {
                name: "default"
            },
            State {
                name: "search"
            }
        ]

        RowLayout {
            spacing: units.gu(0)
            anchors.fill: parent

            Item {
                width: units.gu(6)
                height: width

                Icon {
                    width: units.gu(3)
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
                            if (mainToolBar.state === "search")
                                "qrc:/images/edit-clear.svg"
                            else
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
                            if (mainToolBar.state === "search")
                                mainToolBar.state = "default"
                            else
                                drawer.open()
                        }
                    }

                    visible: !mainView.noZone
                    enabled: !mainView.jobRunning
                }
            }

            Label {
                id: titleLabel
                visible: !searchField.visible
                text: stackView.currentItem != null ? stackView.currentItem.pageTitle : ""
                font.pointSize: units.fs("x-large")
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true

                /* Show sleep timer state */
                Icon {
                    width: units.gu(3)
                    height: width
                    anchors.verticalCenter: parent.Center
                    anchors.right: parent.right
                    source: "qrc:/images/timer.svg"
                    visible: player.sleepTimerEnabled
                    enabled: visible
                }
            }

            TextField {
                id: searchField
                visible: mainToolBar.state === "search" && stackView.depth === 1
                enabled: visible
                Layout.fillWidth: true
                font.pointSize: units.fs("large")
                inputMethodHints: Qt.ImhNoPredictiveText
                placeholderText: qsTr("Search music")

                Connections {
                    target: mainToolBar
                    onStateChanged: {
                        if (mainToolBar.state === "default") {
                            // ensure the search is reset (eg pressing Esc)
                            searchField.text = ""
                        } else {
                            // active focus
                            searchField.forceActiveFocus()
                        }
                    }
                }

                opacity: visible ? 1.0 : 0.0

                Behavior on opacity {
                    NumberAnimation { duration: 250 }
                }
            }

            Item {
                width: units.gu(6)
                height: width

                Icon {
                    width: units.gu(3)
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
                    width: units.gu(3)
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
                            onTriggered: applicationSettingsDialog.open()
                        }
                        MenuItem {
                            text: qsTr("About")
                            font.pointSize: units.fs("medium")
                            onTriggered: aboutDialog.open()
                        }
                    }
                }
            }
        }
    }

    ListModel {
        id: tabs
        ListElement { title: qsTr("My Services"); source: "qrc:/ui/MusicServices.qml"; visible: true }
        ListElement { title: qsTr("Favorites"); source: "qrc:/ui/Favorites.qml"; visible: false }
        ListElement { title: qsTr("Artists"); source: "qrc:/ui/Artists.qml"; visible: false }
        ListElement { title: qsTr("Albums"); source: "qrc:/ui/Albums.qml"; visible: false }
        ListElement { title: qsTr("Genres"); source: "qrc:/ui/Genres.qml"; visible: false }
        ListElement { title: qsTr("Playlists"); source: "qrc:/ui/Playlists.qml"; visible: true }

        function initialIndex() {
            return (settings.tabIndex === -1 ? 0
            : settings.tabIndex > tabs.count - 1 ? tabs.count - 1
            : settings.tabIndex);
        }

        function pushNowPlaying()
        {
            if (!wideAspect) {
                // pop existing page now playing
                if (nowPlayingPage !== null && stackView.pop(nowPlayingPage) === nowPlayingPage) {
                    return;
                }
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

    DialogBase {
        id: applicationSettingsDialog
        title: qsTr("General settings")

        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: {
            settings.style = styleBox.displayText
            scaleBox.acceptedValue = settings.scaleFactor
            applicationSettingsDialog.close()
        }
        onRejected: {
            styleBox.currentIndex = styleBox.styleIndex
            mainView.width = Math.round(scaleBox.acceptedValue * mainView.width / settings.scaleFactor);
            mainView.height = Math.round(scaleBox.acceptedValue * mainView.height / settings.scaleFactor);
            settings.scaleFactor = scaleBox.acceptedValue
            applicationSettingsDialog.close()
        }

        ColumnLayout {
            id: settingsColumn
            spacing: units.gu(1)

            RowLayout {
                spacing: units.gu(1)
                Icon {
                    height: units.gu(3)
                    width: height
                    source: "qrc:/images/font-scalling.svg"
                    anchors.rightMargin: units.gu(1)
                }
                SpinBox {
                    id: fontScaleBox
                    from: 50
                    value: settings.fontScaleFactor * 100
                    to: 200
                    stepSize: 10
                    font.pointSize: units.fs("medium");
                    Layout.fillWidth: true

                    property int decimals: 2
                    property real realValue: value / 100
                    property real acceptedValue: 0

                    validator: DoubleValidator {
                        bottom: Math.min(fontScaleBox.from, fontScaleBox.to)
                        top:  Math.max(fontScaleBox.from, fontScaleBox.to)
                    }

                    textFromValue: function(value, locale) {
                        return Number(value / 100).toLocaleString(locale, 'f', fontScaleBox.decimals)
                    }

                    valueFromText: function(text, locale) {
                        return Number.fromLocaleString(locale, text) * 100
                    }

                    Component.onCompleted: {
                        acceptedValue = realValue;
                    }

                    onValueModified: {
                        settings.fontScaleFactor = realValue
                    }
                }
            }

            RowLayout {
                spacing: units.gu(1)
                Icon {
                    height: units.gu(3)
                    width: height
                    source: "qrc:/images/graphic-scalling.svg"
                }
                SpinBox {
                    id: scaleBox
                    from: 50
                    value: settings.scaleFactor * 100
                    to: 400
                    stepSize: 10
                    font.pointSize: units.fs("medium");
                    Layout.fillWidth: true

                    property int decimals: 2
                    property real realValue: value / 100
                    property real acceptedValue: 0

                    validator: DoubleValidator {
                        bottom: Math.min(scaleBox.from, scaleBox.to)
                        top:  Math.max(scaleBox.from, scaleBox.to)
                    }

                    textFromValue: function(value, locale) {
                        return Number(value / 100).toLocaleString(locale, 'f', scaleBox.decimals)
                    }

                    valueFromText: function(text, locale) {
                        return Number.fromLocaleString(locale, text) * 100
                    }

                    Component.onCompleted: {
                        acceptedValue = realValue;
                    }

                    onValueModified: {
                        mainView.width = Math.round(realValue * mainView.width / settings.scaleFactor);
                        mainView.height = Math.round(realValue * mainView.height / settings.scaleFactor);
                        settings.scaleFactor = realValue
                    }
                }
            }

            RowLayout {
                spacing: units.gu(1)
                Layout.fillWidth: true
                Label {
                    text: qsTr("Style")
                    font.pointSize: units.fs("medium");
                }
                ComboBox {
                    id: styleBox
                    property int styleIndex: -1
                    model: AvailableStyles
                    Component.onCompleted: {
                        styleIndex = find(settings.style, Qt.MatchFixedString)
                        if (styleIndex !== -1)
                            currentIndex = styleIndex
                    }
                    Layout.fillWidth: true
                    font.pointSize: units.fs("medium");
                    popup {
                        font.pointSize: units.fs("medium");
                    }
                }
            }

            Label {
                text: qsTr("Restart is required")
                font.pointSize: units.fs("medium")
                color: "red"
                opacity: styleBox.currentIndex !== styleBox.styleIndex ||
                         scaleBox.realValue !== scaleBox.acceptedValue ? 1.0 : 0.0
                horizontalAlignment: Label.AlignHCenter
                verticalAlignment: Label.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    DialogBase {
        id: aboutDialog
        title: qsTr("About")

        standardButtons: Dialog.Close

        contentSpacing: units.gu(0.5)

        Text {
            width: aboutDialog.availableWidth
            text: qsTr("The project has started in 2015 and is intented to make a fast and smart controller for your SONOS devices."
                       + " You can browse your music library and play track or radio on any zones."
                       + " You can manage grouping zones, queue, and playlists, and fully control the playback.")
            wrapMode: Label.Wrap
            font.pointSize: units.fs("medium")
        }
        Text {
            width: aboutDialog.availableWidth
            text: qsTr("Author: %1").arg("Jean-Luc Barriere")
            font.pointSize: units.fs("medium")
        }
        Text {
            width: aboutDialog.availableWidth
            text: qsTr("Version: %1").arg(versionString)
            font.pointSize: units.fs("medium")
        }
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
        height: status === Loader.Ready ? item.height : 0
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
