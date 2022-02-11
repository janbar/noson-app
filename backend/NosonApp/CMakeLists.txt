cmake_minimum_required(VERSION 3.8.2)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

option(QT_STATICPLUGIN "Build a static plugin" OFF)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

if(UNIX AND NOT APPLE)
#    include( FindPkgConfig )
#    pkg_check_modules(DBUS QUIET dbus-1)
    find_library(DBUS_LIBRARIES dbus-1)
    if(DBUS_LIBRARIES)
        find_package(Qt5DBus REQUIRED)
        set(HAVE_DBUS 1)
        add_definitions(-DHAVE_DBUS)
        message(STATUS "Enable interface MPRIS")
    endif()
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

###############################################################################
# configure
include_directories(${noson_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(QT_STATICPLUGIN)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -Muri=NosonApp)
    add_definitions(-DQT_PLUGIN)
    add_definitions(-DQT_STATICPLUGIN)
endif()

if (MSVC)
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W3 /Od /RTC1")
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /EHsc /nologo")
  set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /O2 /EHsc /nologo")
  set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /EHsc /nologo")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /Od /RTC1 /EHsc /nologo")
endif ()

set(
    NosonApp_SOURCES
    plugin.cpp
    sonos.cpp
    player.cpp
    tracksmodel.cpp
    queuemodel.cpp
    playlistsmodel.cpp
    zonesmodel.cpp
    renderingmodel.cpp
    roomsmodel.cpp
    favoritesmodel.cpp
    servicesmodel.cpp
    mediamodel.cpp
    qmlfilterbehavior.cpp
    qmlsortbehavior.cpp
    qmlsortfiltermodel.cpp
    allservicesmodel.cpp
    alarmsmodel.cpp
    future.cpp
    librarymodel.cpp
)

set(
    NosonApp_HEADERS
    plugin.h
    tools.h
    locked.h
    cppdef.h
    sonos.h
    player.h
    listmodel.h
    tracksmodel.h
    queuemodel.h
    playlistsmodel.h
    zonesmodel.h
    renderingmodel.h
    roomsmodel.h
    favoritesmodel.h
    servicesmodel.h
    mediamodel.h
    qmlfilterbehavior.h
    qmlsortbehavior.h
    qmlsortfiltermodel.h
    allservicesmodel.h
    alarmsmodel.h
    future.h
    librarymodel.h
)

if(HAVE_DBUS)
    set(NosonApp_SOURCES ${NosonApp_SOURCES} dbus/mpris2.cpp)
    set(NosonApp_HEADERS ${NosonApp_HEADERS} dbus/mpris2.h)
    qt5_add_dbus_adaptor(NosonApp_SOURCES
        dbus/org.mpris.MediaPlayer2.xml
        dbus/mpris2.h nosonapp::Mpris2 mpris2_root Mpris2Root)
    qt5_add_dbus_adaptor(NosonApp_SOURCES
        dbus/org.mpris.MediaPlayer2.Player.xml
        dbus/mpris2.h nosonapp::Mpris2 mpris2_player Mpris2Player)
endif()

if(QT_STATICPLUGIN)
    add_library(NosonApp STATIC ${NosonApp_SOURCES} ${NosonApp_HEADERS})
else()
    add_library(NosonApp MODULE ${NosonApp_SOURCES} ${NosonApp_HEADERS})
endif()

if(NOT noson_FOUND)
    add_dependencies (NosonApp noson)
endif()

set_target_properties(NosonApp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QML_IMPORT_DIRECTORY}/NosonApp)
target_link_libraries(NosonApp ${noson_LIBRARIES} Qt5::Gui Qt5::Qml Qt5::Quick)

if(HAVE_DBUS)
  target_link_libraries(NosonApp Qt5::DBus)
endif()

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonApp-qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${QML_IMPORT_DIRECTORY}/NosonApp/qmldir
    DEPENDS ${QMLFILES}
)

if(NOT QT_STATICPLUGIN)
  # Install plugin file
  MESSAGE(STATUS "PlugIns install path: ${PLUGINS_DIR}")
  install(TARGETS NosonApp DESTINATION ${PLUGINS_DIR}/NosonApp/)
  install(FILES   qmldir DESTINATION ${PLUGINS_DIR}/NosonApp/)
endif()
