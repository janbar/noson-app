cmake_minimum_required(VERSION 3.8.2)

OPTION (BUILD_LIBNOSON "Build libnoson from sources." ON)

find_package(Qt5Core REQUIRED)
message(STATUS "Found Qt5 version ${Qt5Core_VERSION}")

if(NOT BUILD_DEPENDENCIES)
    if(NOT BUILD_LIBNOSON)
        find_package(noson 2.6 REQUIRED)
    endif()
endif()
if(NOT noson_FOUND)
    if(Qt5Core_VERSION VERSION_LESS 5.12.4)
        set(OPENSSL_DL_ABI "1.0")
    endif()
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
