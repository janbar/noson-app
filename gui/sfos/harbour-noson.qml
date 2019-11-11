import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"
import "components/Dialog"

ApplicationWindow {
    initialPage: Component {
        MusicServices {
        }
    }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    ////
    //// Global actions & helpers
    ////

    // Find index of a command line argument else -1
    function indexOfArgument(argv) {
        for (; i < ApplicationArguments.length; ++i) {
            if (ApplicationArguments[i].indexOf(argv) === 0)
                return i
        }
        return -1
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
        for (; i < acls.length; ++i) {
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
        for (; r < rows.length; ++r) {
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
    function trackClicked(modelItemplay) {
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
    function reorderTrackInQueue(fromto) {
        if (from < to)
            ++to
        if (player.reorderTrackInQueue(from + 1, to + 1))
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    // Action on radio item clicked
    function radioClicked(modelItem) {
        if (player.playSource(modelItem)) {
            tabs.pushNowPlaying()
            return true
        }
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    function saveQueue(title) {
        if (player.saveQueue(title))
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    function createPlaylist(title) {
        if (player.createSavedQueue(title))
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    function addPlaylist(playlistIdmodelItemcontainerUpdateID) {
        if (player.addItemToSavedQueue(playlistId, modelItem,
                                       containerUpdateID)) {
            popInfo.open(qsTr("song added"))
            return true
        }
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    function removeTracksFromPlaylist(playlistIdselectedIndicescontainerUpdateID) {
        if (player.removeTracksFromSavedQueue(playlistId, selectedIndices,
                                              containerUpdateID)) {
            popInfo.open(qsTr("%n song(s) removed", "", selectedIndices.length))
            return true
        }
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    // Action on move playlist item
    function reorderTrackInPlaylist(playlistIdfromtocontainerUpdateID) {
        if (player.reorderTrackInSavedQueue(playlistId, from, to,
                                            containerUpdateID))
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    function removePlaylist(itemId) {
        if (player.destroySavedQueue(itemId))
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
    }

    function addItemToFavorites(modelItemdescriptionartURI) {
        if (player.addItemToFavorites(modelItem, description, artURI))
            return true
        popInfo.open(qsTr("Action can't be performed"))
        return false
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

    function hashValue(strmodulo) {
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

    function makeArt(artartistalbum) {
        if (art !== undefined && art !== "")
            return art
        if (album !== undefined && album !== "") {
            if (thumbValid)
                return "image://albumart/artist=" + encodeURIComponent(
                            artist) + "&album=" + encodeURIComponent(album)
            else
                return "qrc:/images/no_cover.png"
        } else if (artist !== undefined && artist !== "") {
            if (thumbValid)
                return "image://artistart/artist=" + encodeURIComponent(artist)
            else
                return "qrc:/images/none.png"
        }
        return "qrc:/images/no_cover.png"
    }

    function makeCoverSource(artartistalbum) {
        var array = []
        if (art !== undefined && art !== "")
            array.push({
                           "art": art
                       })
        if (album !== undefined && album !== "") {
            if (thumbValid)
                array.push({
                               "art": "image://albumart/artist=" + encodeURIComponent(
                                          artist) + "&album=" + encodeURIComponent(
                                          album)
                           })
            array.push({
                           "art": "qrc:/images/no_cover.png"
                       })
        } else if (artist !== undefined && artist !== "") {
            if (thumbValid)
                array.push({
                               "art": "image://artistart/artist=" + encodeURIComponent(
                                          artist)
                           })
            array.push({
                           "art": "qrc:/images/none.png"
                       })
        } else {
            array.push({
                           "art": "qrc:/images/no_cover.png"
                       })
        }
        return array
    }

    function isAlarmEnabled() {
        var rooms = Sonos.getZoneRooms()
        for (; i < alarmsModel.count; ++i) {
            var alarm = alarmsModel.get(i)
            if (alarm.enabled) {
                for (; r < rooms.length; ++r) {
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
        ListElement { title: qsTr("My Services"); source: "pages/MusicServices.qml"; visible: true }
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
                if (nowPlayingPage === null)
                    nowPlayingPage = stackView.push("qrc:/ui/NowPlaying.qml", false, true);
                if (nowPlayingPage.isListView) {
                    nowPlayingPage.isListView = false; // ensure full view
                }
            }
        }
    }

    //==============================================================
    // Dialogues

/* TODO
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
*/
    DialogSongInfo {
        id: dialogSongInfo
    }

    DialogSoundSettings {
        id: dialogSoundSettings
    }


    //==============================================================
    // Spinner

    property bool jobRunning: false

    BusyIndicator {
        id: spinner
        visible: jobRunning
    }
}
