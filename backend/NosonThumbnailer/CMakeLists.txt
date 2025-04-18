cmake_minimum_required(VERSION 3.8.2...3.20)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

option(QT_STATICPLUGIN "Build a static plugin" OFF)

find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

###############################################################################
# configure
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

if(QT_STATICPLUGIN)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -Muri=NosonThumbnailer)
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
  NosonThumbnailer_SOURCES
  plugin.cpp
  albumartgenerator.cpp
  artistartgenerator.cpp
  thumbnailerimageresponse.cpp
  thumbnailer/thumbnailer.cpp
  thumbnailer/thumbnailerjob.cpp
  thumbnailer/ratelimiter.cpp
  thumbnailer/netrequest.cpp
  thumbnailer/netmanager.cpp
  thumbnailer/diskcachemanager.cpp
  thumbnailer/artistinfo.cpp
  thumbnailer/albuminfo.cpp
  thumbnailer/abstractapi.cpp
  thumbnailer/tinyxml2.cpp
  thumbnailer/xmldict.cpp
  thumbnailer/jsonparser.cpp
  thumbnailer/lastfm/lastfm.cpp
  thumbnailer/lastfm/lfm-artistinfo.cpp
  thumbnailer/lastfm/lfm-albuminfo.cpp
  thumbnailer/deezer/deezer.cpp
  thumbnailer/deezer/deezer-artistinfo.cpp
  thumbnailer/deezer/deezer-albuminfo.cpp
)

set(
  NosonThumbnailer_HEADERS
  plugin.h
  albumartgenerator.h
  artistartgenerator.h
  thumbnailerimageresponse.h
  thumbnailer/thumbnailer.h
  thumbnailer/thumbnailerjob.h
  thumbnailer/ratelimiter.h
  thumbnailer/netrequest.h
  thumbnailer/netmanager.h
  thumbnailer/diskcachemanager.h
  thumbnailer/artistinfo.h
  thumbnailer/albuminfo.h
  thumbnailer/abstractapi.h
  thumbnailer/jsonparser.h
  thumbnailer/lastfm/lastfm.h
  thumbnailer/deezer/deezer.h
)

if(QT_STATICPLUGIN)
    add_library(NosonThumbnailer STATIC ${NosonThumbnailer_SOURCES} ${NosonThumbnailer_HEADERS})
else()
    add_library(NosonThumbnailer MODULE ${NosonThumbnailer_SOURCES} ${NosonThumbnailer_HEADERS})
endif()

set_target_properties(NosonThumbnailer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QML_IMPORT_DIRECTORY}/NosonThumbnailer)
target_link_libraries(NosonThumbnailer Qt5::Qml Qt5::Quick)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonThumbnailer-qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${QML_IMPORT_DIRECTORY}/NosonThumbnailer/qmldir
    DEPENDS ${QMLFILES}
)

if(NOT QT_STATICPLUGIN)
  # Install plugin file
  MESSAGE(STATUS "PlugIns install path: ${PLUGINS_DIR}")
  install(TARGETS NosonThumbnailer DESTINATION ${PLUGINS_DIR}/NosonThumbnailer/)
  install(FILES   qmldir DESTINATION ${PLUGINS_DIR}/NosonThumbnailer/)
endif()
