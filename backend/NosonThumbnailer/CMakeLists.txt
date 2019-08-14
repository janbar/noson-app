cmake_minimum_required(VERSION 3.1.0)
# Automatically create moc files
set(CMAKE_AUTOMOC ON)

option(QT_STATICPLUGIN "Build a static plugin" OFF)

find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

###############################################################################
# configure
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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
  NosonThumbnailer_SRCS
  plugin.cpp
  plugin.h
  albumartgenerator.cpp
  albumartgenerator.h
  artistartgenerator.cpp
  artistartgenerator.h
  thumbnailerimageresponse.cpp
  thumbnailerimageresponse.h
  thumbnailer/thumbnailer.cpp
  thumbnailer/thumbnailer.h
  thumbnailer/thumbnailerjob.cpp
  thumbnailer/thumbnailerjob.h
  thumbnailer/ratelimiter.cpp
  thumbnailer/ratelimiter.h
  thumbnailer/netrequest.cpp
  thumbnailer/netrequest.h
  thumbnailer/netmanager.cpp
  thumbnailer/netmanager.h
  thumbnailer/diskcachemanager.cpp
  thumbnailer/diskcachemanager.h
  thumbnailer/artistinfo.cpp
  thumbnailer/artistinfo.h
  thumbnailer/albuminfo.cpp
  thumbnailer/albuminfo.h
  thumbnailer/abstractapi.cpp
  thumbnailer/abstractapi.h
  thumbnailer/tinyxml2.cpp
  thumbnailer/xmldict.cpp
  thumbnailer/jsonparser.cpp
  thumbnailer/jsonparser.h
  thumbnailer/lastfm/lastfm.cpp
  thumbnailer/lastfm/lastfm.h
  thumbnailer/lastfm/lfm-artistinfo.cpp
  thumbnailer/lastfm/lfm-albuminfo.cpp
  thumbnailer/deezer/deezer.cpp
  thumbnailer/deezer/deezer.h
  thumbnailer/deezer/deezer-artistinfo.cpp
  thumbnailer/deezer/deezer-albuminfo.cpp
)

if(QT_STATICPLUGIN)
    add_library(NosonThumbnailer STATIC ${NosonThumbnailer_SRCS})
else()
    add_library(NosonThumbnailer MODULE ${NosonThumbnailer_SRCS})
endif()

set_target_properties(NosonThumbnailer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QML_IMPORT_DIRECTORY}/NosonThumbnailer)
target_link_libraries(NosonThumbnailer Qt5::Qml Qt5::Quick)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonThumbnailer-qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${QML_IMPORT_DIRECTORY}/NosonThumbnailer/qmldir
    DEPENDS ${QMLFILES}
)

# Install plugin file
MESSAGE(STATUS "PlugIns install path: ${PLUGINS_DIR}")
install(TARGETS NosonThumbnailer DESTINATION ${PLUGINS_DIR}/NosonThumbnailer/)
install(FILES   qmldir DESTINATION ${PLUGINS_DIR}/NosonThumbnailer/)
