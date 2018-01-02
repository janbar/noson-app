find_package(ZLIB)
find_package(OpenSSL)

# Provides noson
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/noson/noson
    EXCLUDE_FROM_ALL
)
