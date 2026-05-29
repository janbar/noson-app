cmake_minimum_required(VERSION 3.8.2...3.20)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(QML_IMPORT_PATH ${CMAKE_CURRENT_BINARY_DIR}/qml CACHE STRING "QML Import path" FORCE)

add_subdirectory(NosonApp)
add_subdirectory(NosonThumbnailer)
add_subdirectory(NosonMediaScanner)

# build the cli tool for desktop only
if(NOT ANDROID)
    add_subdirectory(cli)
endif()
