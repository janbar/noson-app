cmake_minimum_required(VERSION 3.8.2)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Automatically create moc files
set(CMAKE_AUTOMOC ON)

option(QT_STATICPLUGIN "Build a static plugin" OFF)

option(DISABLE_ID3PARSER "Disable ID3 parser" OFF)
option(DISABLE_MP4PARSER "Disable MP4 parser" OFF)
option(DISABLE_OGGPARSER "Disable OGG parser" OFF)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

###############################################################################
# configure
if(DISABLE_ID3PARSER)
  add_definitions("-DDISABLE_ID3PARSER")
endif()
if(DISABLE_MP4PARSER)
  add_definitions("-DDISABLE_MP4PARSER")
endif()
if(DISABLE_OGGPARSER)
  add_definitions("-DDISABLE_OGGPARSER")
endif()

if(QT_STATICPLUGIN)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -Muri=NosonMediaScanner)
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
  NosonMediaScanner_SOURCES
  plugin.cpp
  mediascanner.cpp
  mediascannerengine.cpp
  mediarunnable.cpp
  mediaextractor.cpp
  flacparser.cpp
  id3parser.cpp
  m4aparser.cpp
  oggparser.cpp
  listmodel.cpp
  byteorder.cpp
  aggregate/artists.cpp
  aggregate/genres.cpp
  aggregate/albums.cpp
  aggregate/tracks.cpp
)

set(
  NosonMediaScanner_HEADERS
  plugin.h
  mediascanner.h
  mediascannerengine.h
  mediarunnable.h
  mediaextractor.h
  flacparser.h
  id3parser.h
  id3v1genres.h
  m4aparser.h
  oggparser.h
  listmodel.h
  mediaparser.h
  mediafile.h
  mediainfo.h
  byteorder.h
  locked.h
  tools.h
  packed.h
  aggregate/aggregate.h
  aggregate/artists.h
  aggregate/genres.h
  aggregate/albums.h
  aggregate/tracks.h
)

include_directories (${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

if(QT_STATICPLUGIN)
    add_library(NosonMediaScanner STATIC ${NosonMediaScanner_SOURCES})
else()
    add_library(NosonMediaScanner MODULE ${NosonMediaScanner_SOURCES})
endif()

set_target_properties(NosonMediaScanner PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QML_IMPORT_DIRECTORY}/NosonMediaScanner)
target_link_libraries(NosonMediaScanner Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonMediaScanner-qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${QML_IMPORT_DIRECTORY}/NosonMediaScanner/qmldir
    DEPENDS ${QMLFILES}
)

if(NOT QT_STATICPLUGIN)
  # Install plugin file
  MESSAGE(STATUS "PlugIns install path: ${PLUGINS_DIR}")
  install(TARGETS NosonMediaScanner DESTINATION ${PLUGINS_DIR}/NosonMediaScanner/)
  install(FILES   qmldir DESTINATION ${PLUGINS_DIR}/NosonMediaScanner/)
endif()
