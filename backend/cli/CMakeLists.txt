cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

###############################################################################
# configure
include_directories(
    ${noson_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/noson/noson/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(
  noson-cli-sources
  noson.cpp
)

add_executable (noson-cli ${noson-cli-sources})

if(NOT noson_FOUND)
    add_dependencies(noson-cli noson)
endif()

set(noson-cli_LIBRARIES ${noson_LIBRARIES})

if(MSVC)
    list(APPEND noson-cli_LIBRARIES ws2_32)
endif()

target_link_libraries(noson-cli ${noson-cli_LIBRARIES})

install(TARGETS noson-cli DESTINATION ${PLUGINS_DIR}/)
