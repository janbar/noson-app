cmake_minimum_required(VERSION 3.1.0)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

###############################################################################
# configure
set(CMAKE_CXX_STANDARD 11)

include_directories(
    ${noson_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/noson/noson/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(
    NosonApp-static_SRCS
    tools.h
    sonos.cpp
    sonos.h
    player.cpp
    player.h
    listmodel.cpp
    listmodel.h
    albumsmodel.cpp
    albumsmodel.h
    artistsmodel.cpp
    artistsmodel.h
    genresmodel.cpp
    genresmodel.h
    tracksmodel.cpp
    tracksmodel.h
    queuemodel.cpp
    queuemodel.h
    playlistsmodel.cpp
    playlistsmodel.h
    zonesmodel.cpp
    zonesmodel.h
    renderingmodel.cpp
    renderingmodel.h
    roomsmodel.cpp
    roomsmodel.h
    favoritesmodel.cpp
    favoritesmodel.h
    servicesmodel.cpp
    servicesmodel.h
    mediamodel.cpp
    mediamodel.h
    qmlfilterbehavior.cpp
    qmlfilterbehavior.h
    qmlsortbehavior.cpp
    qmlsortbehavior.h
    qmlsortfiltermodel.cpp
    qmlsortfiltermodel.h
    allservicesmodel.cpp
    allservicesmodel.h
    alarmsmodel.cpp
    alarmsmodel.h
    composersmodel.cpp
    composersmodel.h
)

add_library(NosonApp-static STATIC ${NosonApp-static_SRCS})

#if(NOT noson_FOUND)
    add_dependencies (NosonApp-static noson)
#endif()

target_link_libraries(NosonApp-static ${noson_LIBRARIES} Qt5::Gui Qt5::Qml Qt5::Quick)
