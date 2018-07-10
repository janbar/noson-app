cmake_minimum_required(VERSION 3.1.0)

if(NOT BUILD_DEPENDENCIES)
    find_package(noson QUIET)
endif()
if(NOT noson_FOUND)
    set(noson_INCLUDE_DIRS
        "${CMAKE_BINARY_DIR}/backend/lib/noson/noson/public/noson"
        "${CMAKE_BINARY_DIR}/backend/lib/noson/noson/public"
    )
    set(noson_LIBRARIES noson)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/backend/lib/noson/noson/src
    ${noson_INCLUDE_DIRS}
)

set(
  noson-cli-sources
  noson.cpp
)

add_executable (noson-cli ${noson-cli-sources})

if(NOT noson_FOUND)
    add_dependencies(noson-cli noson)
endif()

if(MSVC)
    target_link_libraries (noson-cli ${noson_LIBRARIES} ws2_32)
else()
    target_link_libraries (noson-cli ${noson_LIBRARIES})
endif()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/noson-cli
    DESTINATION ${PLUGINS_DIR})
