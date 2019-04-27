cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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
if(DISABLE_MP4PARSER)
  add_definitions("-DDISABLE_MP4PARSER")
endif()

if(QT_STATICPLUGIN)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -Muri=NosonMediaScanner)
    add_definitions(-DQT_PLUGIN)
    add_definitions(-DQT_STATICPLUGIN)
endif()

set(
  NosonMediaScanner_SRCS
  plugin.cpp
  plugin.h
  mediascanner.cpp
  mediascanner.h
  mediascannerengine.cpp
  mediascannerengine.h
  mediarunnable.cpp
  mediarunnable.h
  mediaextractor.cpp
  mediaextractor.h
  flacparser.cpp
  flacparser.h
  id3parser.cpp
  id3parser.h
  id3v1genres.h
  m4aparser.cpp
  m4aparser.h
  listmodel.cpp
  listmodel.h
  mediaparser.h
  mediafile.h
  mediainfo.h
  byteorder.cpp
  byteorder.h
  locked.h
  tools.h
  aggregate/aggregate.h
  aggregate/artists.cpp
  aggregate/artists.h
  aggregate/genres.cpp
  aggregate/genres.h
  aggregate/albums.cpp
  aggregate/albums.h
  aggregate/tracks.cpp
  aggregate/tracks.h
)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

if(QT_STATICPLUGIN)
    add_library(NosonMediaScanner STATIC ${NosonMediaScanner_SRCS})
else()
    add_library(NosonMediaScanner MODULE ${NosonMediaScanner_SRCS})
endif()

set_target_properties(NosonMediaScanner PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QML_IMPORT_DIRECTORY}/NosonMediaScanner)
target_link_libraries(NosonMediaScanner Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonMediaScanner-qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${QML_IMPORT_DIRECTORY}/NosonMediaScanner/qmldir
    DEPENDS ${QMLFILES}
)

# Install plugin file
MESSAGE(STATUS "PlugIns install path: ${PLUGINS_DIR}")
install(TARGETS NosonMediaScanner DESTINATION ${PLUGINS_DIR}/NosonMediaScanner/)
install(FILES   qmldir DESTINATION ${PLUGINS_DIR}/NosonMediaScanner/)
