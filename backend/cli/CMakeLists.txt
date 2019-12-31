cmake_minimum_required(VERSION 3.8.2)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

###############################################################################
# configure
include_directories(${noson_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if (MSVC)
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W3 /Od /RTC1")
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /EHsc /nologo")
  set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /O2 /EHsc /nologo")
  set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /EHsc /nologo")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /Od /RTC1 /EHsc /nologo")
endif ()

set(
  noson-cli_SOURCES
  noson.cpp
)

set(
  noson-cli_HEADERS
  tokenizer.h
  builtin.h
)

add_executable (noson-cli ${noson-cli_SOURCES})

if(NOT noson_FOUND)
    add_dependencies(noson-cli noson)
endif()

set(noson-cli_LIBRARIES ${noson_LIBRARIES})

if(MSVC)
    list(APPEND noson-cli_LIBRARIES ws2_32)
endif()

target_link_libraries(noson-cli ${noson-cli_LIBRARIES})

install(TARGETS noson-cli DESTINATION ${PLUGINS_DIR}/)
