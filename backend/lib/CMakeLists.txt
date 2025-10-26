cmake_minimum_required(VERSION 3.8.2...3.20)

option(BUILD_DEPENDENCIES "Build dependencies from sources" OFF)
option(LIBNOSON_GIT_MASTER "Build the development branch of libnoson" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  set(MSVC_DYNAMIC_RUNTIME ON)
endif()

include(DownloadProject)

# Disable install rules
set(SKIP_INSTALL_ALL ON)

# Provides dependency zlib
if(NOT BUILD_DEPENDENCIES)
    find_package(ZLIB QUIET)
endif()
if(NOT ZLIB_FOUND)
    download_project(PROJ zlib
        URL "${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.1.tar.gz"
        URL_HASH MD5=9855b6d802d7fe5b7bd5b196a2271655
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/zlib-src
        ${CMAKE_CURRENT_BINARY_DIR}/zlib-build
        EXCLUDE_FROM_ALL
    )
    set(ZLIB_LIBRARY zlibstatic)
    set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
    set(ZLIB_INCLUDE_DIR "${zlib_BINARY_DIR}")
    set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
    set(ZLIB_FOUND ON)
    export(TARGETS ${ZLIB_LIBRARY} FILE ZLIBTargets.cmake)
    add_library(ZLIB::ZLIB ALIAS ${ZLIB_LIBRARY})
    message(STATUS "Build ZLIB: ${zlib_BINARY_DIR}")
    list(APPEND noson_DEPENDENCIES ${ZLIB_LIBRARY})
endif()

# Provides dependency openssl
if(NOT BUILD_DEPENDENCIES)
    find_package(OpenSSL QUIET)
endif()
if(NOT OPENSSL_FOUND)
    set(OPENSSL_DL_ABI "1.1")
    download_project(PROJ "openssl-${OPENSSL_DL_ABI}"
        GIT_REPOSITORY https://github.com/janbar/openssl-cmake.git
        GIT_TAG 1.1.1w-20250419
        GIT_SHALLOW 1
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    if(ANDROID OR MSVC)
        set(BUILD_SHARED_LIBS ON)
    else()
        set(BUILD_SHARED_LIBS OFF)
    endif()
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/openssl-${OPENSSL_DL_ABI}-src
        ${CMAKE_CURRENT_BINARY_DIR}/openssl-${OPENSSL_DL_ABI}-build
        EXCLUDE_FROM_ALL
    )
    set(OPENSSL_SSL_LIBRARY ssl)
    set(OPENSSL_CRYPTO_LIBRARY crypto)
    set(OPENSSL_INCLUDE_DIR "${openssl_BINARY_DIR}/include" "${openssl_BINARY_DIR}")
    set(OPENSSL_FOUND ON)
    export(TARGETS crypto ssl FILE OpenSSLTargets.cmake)
    add_library(OpenSSL::Crypto ALIAS crypto)
    add_library(OpenSSL::SSL ALIAS ssl)
    message(STATUS "Build OpenSSL: ${openssl_BINARY_DIR}")
    list(APPEND noson_DEPENDENCIES ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})
endif()

# Provides dependency FLAC
if(NOT BUILD_DEPENDENCIES)
    find_package(FLAC)
    find_package(FLACXX)
endif()
if(NOT FLAC_FOUND OR NOT FLACXX_FOUND)
    download_project(PROJ flac
        GIT_REPOSITORY https://github.com/janbar/flac-cmake.git
        GIT_TAG 1.3.2-20250419
        GIT_SHALLOW 1
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}
        UPDATE_DISCONNECTED ON)
    if(ANDROID OR MSVC)
        set(BUILD_SHARED_LIBS ON)
    else()
        set(BUILD_SHARED_LIBS OFF)
    endif()
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/flac-src
        ${CMAKE_CURRENT_BINARY_DIR}/flac-build
        EXCLUDE_FROM_ALL
    )
    set(FLAC_LIBRARIES FLAC)
    set(FLAC_INCLUDE_DIR "${FLAC_SOURCE_DIR}/include")
    set(FLAC_FOUND ON)
    set(FLACXX_LIBRARIES FLACXX)
    set(FLACXX_INCLUDE_DIR "${FLAC_SOURCE_DIR}/include")
    set(FLACXX_FOUND ON)
    message(STATUS "Build FLAC FLAC++: ${FLAC_BINARY_DIR}")
    list(APPEND noson_DEPENDENCIES FLAC FLACXX)
endif()

set(BUILD_SHARED_LIBS OFF)

# Provides dependency noson
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/noson/CMakeLists.txt)
    message(STATUS "Using libnoson submodule")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/noson/noson)
else()
    if(NOT LIBNOSON_GIT_MASTER)
        download_project(PROJ noson
          URL "${CMAKE_CURRENT_SOURCE_DIR}/noson-2.12.11.tar.gz"
          URL_HASH MD5=04c596edd87f0315b9a9f2e7f127a1ed
          PREFIX ${CMAKE_CURRENT_BINARY_DIR}
          UPDATE_DISCONNECTED ON)
    else()
        message(STATUS "Downloading git master libnoson")
        download_project(PROJ noson
            GIT_REPOSITORY https://github.com/janbar/noson.git
            GIT_TAG master
            GIT_SHALLOW 1
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}
            UPDATE_DISCONNECTED OFF)
    endif()
    add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/noson-src/noson
        ${CMAKE_CURRENT_BINARY_DIR}/noson-build
        EXCLUDE_FROM_ALL
    )
endif()

set(noson_LIBRARIES noson
    ${ZLIB_LIBRARIES}
    ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY}
    ${FLAC_LIBRARIES} ${FLACXX_LIBRARIES}               PARENT_SCOPE)
set(noson_INCLUDE_DIRS "${noson_BINARY_DIR}/public"     PARENT_SCOPE)
set(noson_FOUND ON                                      PARENT_SCOPE)
message(STATUS "Build noson: ${noson_BINARY_DIR}")

if(noson_DEPENDENCIES)
    add_dependencies(noson ${noson_DEPENDENCIES})
endif()
