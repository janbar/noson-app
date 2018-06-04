cmake_minimum_required(VERSION 3.1.0)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/../backend/lib/noson/noson/include
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
