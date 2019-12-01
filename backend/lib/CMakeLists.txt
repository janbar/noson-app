cmake_minimum_required(VERSION 3.1.0)

option(BUILD_DEPENDENCIES "Build dependencies from sources" off)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

# Disable install rules
set(SKIP_INSTALL_ALL ON)

# Provides dependency zlib
if(NOT BUILD_DEPENDENCIES)
    find_package(ZLIB QUIET)
endif()
if(NOT ZLIB_FOUND)
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/zlib
        ${CMAKE_CURRENT_BINARY_DIR}/zlib
        EXCLUDE_FROM_ALL
    )
    set(ZLIB_LIBRARIES zlib)
    set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
    set(ZLIB_FOUND ON)
    message(STATUS "Build ZLIB: ${zlib_BINARY_DIR}")
    list(APPEND noson_DEPENDENCIES zlib)
endif()

# Provides dependency openssl
if(NOT BUILD_DEPENDENCIES)
    find_package(OpenSSL QUIET)
endif()
if(NOT OPENSSL_FOUND)
    execute_process (COMMAND git submodule update --init --depth 1 -- openssl
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    if(ANDROID OR MSVC)
        set(BUILD_SHARED_LIBS ON)
    else()
        set(BUILD_SHARED_LIBS OFF)
    endif()
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/openssl
        ${CMAKE_CURRENT_BINARY_DIR}/openssl
        EXCLUDE_FROM_ALL
    )
    set(OPENSSL_SSL_LIBRARY ssl)
    set(OPENSSL_CRYPTO_LIBRARY crypto)
    set(OPENSSL_INCLUDE_DIR "${openssl_BINARY_DIR}/include" "${openssl_BINARY_DIR}")
    set(OPENSSL_FOUND ON)
    message(STATUS "Build OpenSSL: ${openssl_BINARY_DIR}")
    list(APPEND noson_DEPENDENCIES crypto ssl)
endif()

# Provides dependency FLAC
if(NOT BUILD_DEPENDENCIES)
    find_package(FLAC)
    find_package(FLACXX)
endif()
if(NOT FLAC_FOUND OR NOT FLACXX_FOUND)
    execute_process (COMMAND git submodule update --init --depth 1 -- flac
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    if(ANDROID OR MSVC)
        set(BUILD_SHARED_LIBS ON)
    else()
        set(BUILD_SHARED_LIBS OFF)
    endif()
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/flac
        ${CMAKE_CURRENT_BINARY_DIR}/flac
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

# Provides dependency noson
execute_process (COMMAND git submodule update --init --depth 1 -- noson
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/noson/noson
    EXCLUDE_FROM_ALL
)
set(noson_LIBRARIES noson                               PARENT_SCOPE)
set(noson_INCLUDE_DIRS "${noson_BINARY_DIR}/public/noson"
    "${noson_BINARY_DIR}/public"                        PARENT_SCOPE)
set(noson_FOUND ON                                      PARENT_SCOPE)
message(STATUS "Build noson: ${noson_BINARY_DIR}")

if(noson_DEPENDENCIES)
    add_dependencies(noson ${noson_DEPENDENCIES})
endif()
