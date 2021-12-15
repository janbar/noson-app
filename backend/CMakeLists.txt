cmake_minimum_required(VERSION 3.8.2...3.20)

OPTION (BUILD_LIBNOSON "Build libnoson from sources." ON)

find_package(Qt6 REQUIRED)
message(STATUS "Found Qt6 version ${Qt6_VERSION}")

if(NOT BUILD_DEPENDENCIES)
    if(NOT BUILD_LIBNOSON)
        find_package(noson 2.12 REQUIRED)
    endif()
endif()
if(NOT noson_FOUND)
    add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/lib)
endif()

set(QML_IMPORT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qml)
set(QML_IMPORT_PATH ${QML_IMPORT_DIRECTORY} CACHE STRING "QML Import path" FORCE)

add_subdirectory(NosonApp)
add_subdirectory(NosonThumbnailer)
add_subdirectory(NosonMediaScanner)

# build the cli tool for desktop only
if(NOT ANDROID)
    add_subdirectory(cli)
endif()
