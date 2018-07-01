cmake_minimum_required(VERSION 3.1.0)

option(BUILD_DEPENDENCIES "Build dependencies from sources" off)

if(CMAKE_COMPILER_IS_GNUCC)
    add_definitions ("-fPIC")
endif()

# Disable install rules
set(SKIP_INSTALL_ALL ON)

# Provides dependency zlib
if(NOT BUILD_DEPENDENCIES)
        find_package(ZLIB QUIET)
endif()
if(NOT ZLIB_FOUND)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/zlib
        ${CMAKE_CURRENT_BINARY_DIR}/zlib
        EXCLUDE_FROM_ALL
    )
    set(ZLIB_LIBRARIES zlib)
    set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
    set(ZLIB_FOUND ON)
    message(STATUS "Build ZLIB: ${zlib_BINARY_DIR}")
endif()

# Provides dependency openssl
if(NOT BUILD_DEPENDENCIES)
        find_package(OpenSSL QUIET)
endif()
if(NOT OPENSSL_FOUND)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/openssl
        ${CMAKE_CURRENT_BINARY_DIR}/openssl
        EXCLUDE_FROM_ALL
    )
    set(OPENSSL_SSL_LIBRARY ssl)
    set(OPENSSL_CRYPTO_LIBRARY crypto)
    set(OPENSSL_INCLUDE_DIR "${openssl_SOURCE_DIR}/include" "${openssl_BINARY_DIR}")
    set(OPENSSL_FOUND ON)
    message(STATUS "Build OpenSSL: ${openssl_BINARY_DIR}")
endif()

# Provides dependency noson
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/noson/noson
    EXCLUDE_FROM_ALL
)
set(noson_INCLUDE_DIRS "${noson_BINARY_DIR}/public/noson" "${noson_BINARY_DIR}/public" PARENT_SCOPE)
set(noson_LIBRARIES noson PARENT_SCOPE)
message(STATUS "Build noson: ${noson_BINARY_DIR}")
