cmake_minimum_required(VERSION 3.1.0)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Automatically create moc files
set(CMAKE_AUTOMOC ON)

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

set(
  NosonMediaScanner-static_SRCS
  mediascanner.cpp
  mediascannerengine.cpp
  mediarunnable.cpp
  mediaextractor.cpp
  flacparser.cpp
  id3parser.cpp
  listmodel.cpp
  byteorder.cpp
  aggregate/artists.cpp
  aggregate/genres.cpp
  aggregate/albums.cpp
  aggregate/tracks.cpp
)

set(
  NosonMediaScanner-static_HDRS
  mediascanner.h
  mediascannerengine.h
  mediarunnable.h
  mediaextractor.h
  flacparser.h
  id3parser.h
  m4aparser.cpp
  m4aparser.h
  listmodel.h
  mediaparser.h
  mediafile.h
  mediainfo.h
  byteorder.h
  locked.h
  tools.h
  aggregate/aggregate.h
  aggregate/artists.h
  aggregate/genres.h
  aggregate/albums.h
  aggregate/tracks.h
)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_library(NosonMediaScanner-static STATIC ${NosonMediaScanner-static_SRCS} ${NosonMediaScanner-static_HDRS})

target_link_libraries(NosonMediaScanner-static Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick)

set(
  NosonMediaScanner_SRCS
  plugin.cpp
  plugin.h
)

add_library(NosonMediaScanner MODULE ${NosonMediaScanner_SRCS})

set_target_properties(NosonMediaScanner PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QML_IMPORT_DIRECTORY}/NosonMediaScanner)
target_link_libraries(NosonMediaScanner NosonMediaScanner-static)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(NosonMediaScanner-qmldir ALL
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${QML_IMPORT_DIRECTORY}/NosonMediaScanner
    DEPENDS ${QMLFILES}
)

# Install plugin file
MESSAGE(STATUS "PlugIns install path: ${PLUGINS_DIR}")
install(TARGETS NosonMediaScanner DESTINATION ${PLUGINS_DIR}/NosonMediaScanner/)
install(FILES   qmldir DESTINATION ${PLUGINS_DIR}/NosonMediaScanner/)
