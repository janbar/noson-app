cmake_minimum_required(VERSION 3.1.0)

OPTION (BUILD_LIBNOSON "Build libnoson from sources." ON)

if(NOT BUILD_DEPENDENCIES)
    if(NOT BUILD_LIBNOSON)
        find_package(noson 1.14 REQUIRED)
    endif()
endif()
if(NOT noson_FOUND)
    add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/lib)
endif()

set(QML_IMPORT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qml)
set(QML_IMPORT_PATH ${QML_IMPORT_DIRECTORY} CACHE string "QML Import path" FORCE)

add_subdirectory(NosonApp)
add_subdirectory(NosonThumbnailer)
# Disable non-streamable medias
set(DISABLE_MP4PARSER ON)
add_subdirectory(NosonMediaScanner)

# build the cli tool for desktop only
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(cli)
endif()
