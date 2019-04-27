cmake_minimum_required(VERSION 3.1.0)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

option(QT_STATICPLUGIN "Build a static plugin" OFF)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

###############################################################################
# configure
include_directories(
    ${noson_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/noson/noson/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(QT_STATICPLUGIN)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -Muri=NosonApp)
    add_definitions(-DQT_PLUGIN)
    add_definitions(-DQT_STATICPLUGIN)
endif()

set(
    NosonApp_SRCS
    plugin.cpp
    plugin.h
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

if(QT_STATICPLUGIN)
    add_library(NosonApp STATIC ${NosonApp_SRCS})
else()
    add_library(NosonApp MODULE ${NosonApp_SRCS})
endif()

if(NOT noson_FOUND)
    add_dependencies (NosonApp noson)
endif()

set_target_properties(NosonApp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QML_IMPORT_DIRECTORY}/NosonApp)
target_link_libraries(NosonApp ${noson_LIBRARIES} Qt5::Gui Qt5::Qml Qt5::Quick)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonApp-qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${QML_IMPORT_DIRECTORY}/NosonApp/qmldir
    DEPENDS ${QMLFILES}
)

# Install plugin file
MESSAGE(STATUS "PlugIns install path: ${PLUGINS_DIR}")
install(TARGETS NosonApp DESTINATION ${PLUGINS_DIR}/NosonApp/)
install(FILES   qmldir DESTINATION ${PLUGINS_DIR}/NosonApp/)
