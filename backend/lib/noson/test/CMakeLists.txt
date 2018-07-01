cmake_minimum_required (VERSION 2.8.9 FATAL_ERROR)
cmake_policy (VERSION 2.8)

project (nosontest)

###############################################################################
# add definitions
if (MSVC)
  add_definitions ("/D_CRT_SECURE_NO_WARNINGS")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /W3")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W3")
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /Od /RTC1 /EHsc /nologo")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /Od /RTC1 /EHsc /nologo")
endif ()

if (NOT MSVC)
  if (COMPILER_SUPPORTS_CXX11)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif ()
endif ()

###############################################################################
# configure
include_directories (
  ${CMAKE_BINARY_DIR}/noson/public/noson
  ${CMAKE_BINARY_DIR}/noson/public
  ${CMAKE_SOURCE_DIR}/noson/src)

###############################################################################
# add targets
add_executable (nosontest src/test.cpp)
add_dependencies (nosontest noson)
target_link_libraries (nosontest noson)

add_executable (nosontestms src/testms.cpp)
add_dependencies (nosontestms noson)
target_link_libraries (nosontestms noson)

add_executable (intrinsictest src/intrinsictest.cpp)
add_dependencies (intrinsictest noson)
target_link_libraries (intrinsictest noson)

add_executable (sslwgettest src/sslwgettest.cpp)
add_dependencies (sslwgettest noson)
target_link_libraries (sslwgettest noson)
