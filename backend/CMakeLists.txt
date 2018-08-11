cmake_minimum_required(VERSION 3.1.0)

OPTION (BUILD_LIBNOSON "Build libnoson from sources." ON)

if(NOT BUILD_DEPENDENCIES)
    if(NOT BUILD_LIBNOSON)
        find_package(noson 1.8 REQUIRED)
    endif()
endif()
if(NOT noson_FOUND)
    add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/lib)
endif()

set(QML_IMPORT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/qml)

add_subdirectory(NosonApp)
