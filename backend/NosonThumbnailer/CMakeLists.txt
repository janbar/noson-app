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
  thumbnailer/lfm-artistinfo.cpp
  thumbnailer/lfm-artistinfo.h
  thumbnailer/lfm-albuminfo.cpp
  thumbnailer/lfm-albuminfo.h
  thumbnailer/tinyxml2.cpp
  thumbnailer/xmldict.cpp
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
