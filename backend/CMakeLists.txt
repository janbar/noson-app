cmake_minimum_required(VERSION 3.1.0)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/lib)

###############################################################################
# configure
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/lib/noson/noson/include
)

set(
    NosonAppbackend_SRCS
    modules/NosonApp/tools.h
    modules/NosonApp/backend.cpp
    modules/NosonApp/backend.h
    modules/NosonApp/sonos.cpp
    modules/NosonApp/sonos.h
    modules/NosonApp/player.cpp
    modules/NosonApp/player.h
    modules/NosonApp/listmodel.cpp
    modules/NosonApp/listmodel.h
    modules/NosonApp/albumsmodel.cpp
    modules/NosonApp/albumsmodel.h
    modules/NosonApp/artistsmodel.cpp
    modules/NosonApp/artistsmodel.h
    modules/NosonApp/genresmodel.cpp
    modules/NosonApp/genresmodel.h
    modules/NosonApp/tracksmodel.cpp
    modules/NosonApp/tracksmodel.h
    modules/NosonApp/queuemodel.cpp
    modules/NosonApp/queuemodel.h
    modules/NosonApp/playlistsmodel.cpp
    modules/NosonApp/playlistsmodel.h
    modules/NosonApp/zonesmodel.cpp
    modules/NosonApp/zonesmodel.h
    modules/NosonApp/renderingmodel.cpp
    modules/NosonApp/renderingmodel.h
    modules/NosonApp/roomsmodel.cpp
    modules/NosonApp/roomsmodel.h
    modules/NosonApp/favoritesmodel.cpp
    modules/NosonApp/favoritesmodel.h
    modules/NosonApp/servicesmodel.cpp
    modules/NosonApp/servicesmodel.h
    modules/NosonApp/mediamodel.cpp
    modules/NosonApp/mediamodel.h
    modules/NosonApp/qmlfilterbehavior.cpp
    modules/NosonApp/qmlfilterbehavior.h
    modules/NosonApp/qmlsortbehavior.cpp
    modules/NosonApp/qmlsortbehavior.h
    modules/NosonApp/qmlsortfiltermodel.cpp
    modules/NosonApp/qmlsortfiltermodel.h
    modules/NosonApp/allservicesmodel.cpp
    modules/NosonApp/allservicesmodel.h
)

add_library(NosonAppbackend MODULE
    ${NosonAppbackend_SRCS}
)

add_dependencies (NosonAppbackend noson)
target_link_libraries (NosonAppbackend noson)

set_target_properties(NosonAppbackend PROPERTIES
         LIBRARY_OUTPUT_DIRECTORY NosonApp)

qt5_use_modules(NosonAppbackend Gui Qml Quick)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonAppbackend-qmldir ALL
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/modules/NosonApp/qmldir ${CMAKE_CURRENT_BINARY_DIR}/NosonApp
    DEPENDS ${QMLFILES}
)

# Install plugin file
MESSAGE(STATUS "PlugIns install path: ${PLUGINS_DIR}")
install(TARGETS NosonAppbackend DESTINATION ${PLUGINS_DIR}/NosonApp/)
install(FILES   modules/NosonApp/qmldir DESTINATION ${PLUGINS_DIR}/NosonApp/)
