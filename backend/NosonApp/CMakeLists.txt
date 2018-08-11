cmake_minimum_required(VERSION 3.1.0)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

###############################################################################
# configure
include_directories(
    ${noson_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/noson/noson/src
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

if(NOT noson_FOUND)
    add_dependencies (NosonApp-static noson)
endif()

target_link_libraries(NosonApp-static ${noson_LIBRARIES}
    Qt5::Gui Qt5::Qml Qt5::Quick)

set(
    NosonApp_SRCS
    plugin.cpp
    plugin.h
)

add_library(NosonApp MODULE ${NosonApp_SRCS})

set_target_properties(NosonApp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QML_IMPORT_DIRECTORY}/NosonApp)
target_link_libraries(NosonApp NosonApp-static)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonApp-qmldir ALL
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${QML_IMPORT_DIRECTORY}/NosonApp
    DEPENDS ${QMLFILES}
)

# Install plugin file
MESSAGE(STATUS "PlugIns install path: ${PLUGINS_DIR}")
install(TARGETS NosonApp DESTINATION ${PLUGINS_DIR}/NosonApp/)
install(FILES   qmldir DESTINATION ${PLUGINS_DIR}/NosonApp/)
