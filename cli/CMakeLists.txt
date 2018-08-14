cmake_minimum_required(VERSION 2.8.9)

set(noson_INCLUDE_DIRS
    "${CMAKE_BINARY_DIR}/backend/lib/noson/noson/public/noson"
    "${CMAKE_BINARY_DIR}/backend/lib/noson/noson/public"
)

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

if(MSVC)
  target_link_libraries (noson-cli noson ws2_32)
else()
  target_link_libraries (noson-cli noson)
endif()

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/noson-cli
  DESTINATION ${PLUGINS_DIR})
