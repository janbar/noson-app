option(BUILD_DEPENDENCIES "Build dependencies from sources" off)

if(CMAKE_COMPILER_IS_GNUCC)
  add_definitions ("-fPIC")
endif()

# Provides dependency zlib
if(NOT BUILD_DEPENDENCIES)
    find_package(ZLIB)
endif()
if(NOT ZLIB_FOUND)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/zlib
        ${CMAKE_CURRENT_BINARY_DIR}/zlib
        EXCLUDE_FROM_ALL
    )
    set(ZLIB_LIBRARIES zlib)
    set(ZLIB_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/zlib
        ${CMAKE_CURRENT_BINARY_DIR}/zlib
    )
    set(ZLIB_FOUND ON)
    message(STATUS "Build ZLIB: ${CMAKE_CURRENT_BINARY_DIR}/zlib")
endif()

# Provides dependency openssl
if(NOT BUILD_DEPENDENCIES)
    find_package(OpenSSL)
endif()
if(NOT OPENSSL_FOUND)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/openssl
        ${CMAKE_CURRENT_BINARY_DIR}/openssl
        EXCLUDE_FROM_ALL
    )
    set(OPENSSL_SSL_LIBRARY ssl)
    set(OPENSSL_CRYPTO_LIBRARY crypto)
    set(OPENSSL_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/openssl/include
        ${CMAKE_CURRENT_BINARY_DIR}/openssl
    )
    set(OPENSSL_FOUND ON)
    message(STATUS "Build OpenSSL: ${CMAKE_CURRENT_BINARY_DIR}/openssl")
endif()

# Provides noson
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/noson/noson
    EXCLUDE_FROM_ALL
)
