cmake_minimum_required(VERSION 3.1.0)

project(FLAC)
project(FLACXX)

enable_language(C)
enable_language(CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

option(ENABLE_OGG "Enbale build with OGG library" OFF)

set(PACKAGE_NAME "flac")
set(PACKAGE_VERSION "1.3.2")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(FLAC_LIB_VERSION "8.3.0")
set(FLAC_LIB_SOVERSION 8)
set(FLACXX_LIB_VERSION "6.3.0")
set(FLACXX_LIB_SOVERSION 6)

###############################################################################
# configure

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckTypeSize)
include(CheckSymbolExists)
include(TestBigEndian)

check_type_size       ("int"            SIZEOF_INT)
check_type_size       ("off_t"          SIZEOF_OFF_T)
check_include_file    ("stdint.h"       HAVE_STDINT_H)
check_include_file    ("byteswap.h"     HAVE_BYTESWAP_H)
check_include_file    ("cpuid.h"        HAVE_CPUID_H)
check_include_file    ("inttypes.h"     HAVE_INTTYPES_H)
check_include_file    ("unistd.h"       HAVE_UNISTD_H)
check_include_file    ("sys/param.h"    HAVE_SYS_PARAM_H)
check_symbol_exists   ("lround"         "math.h;tgmath.h" HAVE_LROUND)
check_symbol_exists   ("bswap_16"       "byteswap.h" HAVE_BSWAP16)
check_symbol_exists   ("bswap_32"       "byteswap.h" HAVE_BSWAP32)
check_symbol_exists   ("bswap_64"       "byteswap.h" HAVE_BSWAP64)
check_symbol_exists   ("iconv"          "iconv.h" HAVE_ICONV)
check_function_exists ("clock_gettime"  HAVE_CLOCK_GETTIME)
check_function_exists ("getopt_long"    HAVE_GETOPT_LONG)

test_big_endian(CPU_IS_BIG_ENDIAN)
if(${CPU_IS_BIG_ENDIAN} EQUAL 0)
    set(CPU_IS_LITTLE_ENDIAN 1)
else()
    set(CPU_IS_LITTLE_ENDIAN 0)
endif()

set(FLAC__NO_ASM 1)

if(ENABLE_OGG)
    set(FLAC__HAS_OGG 1)
else()
    set(FLAC__HAS_OGG 0)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(FLAC__SYS_DARWIN 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(FLAC__SYS_LINUX 1)
endif()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(FLAC__CPU_X86_64 1)
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "amd64")
    set(FLAC__CPU_X86_64 1)
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
    set(FLAC__CPU_IA32 1)
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386")
    set(FLAC__CPU_IA32 1)
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
    set(FLAC__CPU_IA32 1)
endif()

add_definitions(-DHAVE_CONFIG_H)
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in"
                "${CMAKE_CURRENT_BINARY_DIR}/config.h" )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libFLAC/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(
    FLAC_SRCS
    src/libFLAC/bitmath.c
    src/libFLAC/bitreader.c
    src/libFLAC/bitwriter.c
    src/libFLAC/cpu.c
    src/libFLAC/crc.c
    src/libFLAC/fixed.c
    src/libFLAC/fixed_intrin_sse2.c
    src/libFLAC/fixed_intrin_ssse3.c
    src/libFLAC/float.c
    src/libFLAC/format.c
    src/libFLAC/lpc.c
    src/libFLAC/lpc_intrin_avx2.c
    src/libFLAC/lpc_intrin_sse2.c
    src/libFLAC/lpc_intrin_sse41.c
    src/libFLAC/lpc_intrin_sse.c
    src/libFLAC/md5.c
    src/libFLAC/memory.c
    src/libFLAC/metadata_iterators.c
    src/libFLAC/metadata_object.c
    src/libFLAC/stream_decoder.c
    src/libFLAC/stream_encoder.c
    src/libFLAC/stream_encoder_framing.c
    src/libFLAC/stream_encoder_intrin_avx2.c
    src/libFLAC/stream_encoder_intrin_sse2.c
    src/libFLAC/stream_encoder_intrin_ssse3.c
    src/libFLAC/window.c
)

set(
    windows_unicode_compat_SRCS
    src/libFLAC/windows_unicode_filenames.c
)

set(
    extra_ogg_SRCS
    src/libFLAC/ogg_decoder_aspect.c
    src/libFLAC/ogg_encoder_aspect.c
    src/libFLAC/ogg_helper.c
    src/libFLAC/ogg_mapping.c
)

if(NOT MSVC)
    set(FLAC_link_libraries m)
else()
    include(MSVCRuntime)
    configure_msvc_runtime()
    set(extra_SRCS ${windows_unicode_compat_SRCS})
    set(FLAC_link_libraries ws2_32)
    add_definitions(-DFLAC__NO_DLL)
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W3 /Od /RTC1")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /EHsc /nologo")
    set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /O2 /EHsc /nologo")
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /EHsc /nologo")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /Od /RTC1 /EHsc /nologo")
endif()

if(ENABLE_OGG)
    find_library(ogg_LIBRARIES ogg)
    list(APPEND extra_SRCS ${extra_ogg_SRCS})
    list(APPEND FLAC_link_libraries ogg)
endif()

if(BUILD_SHARED_LIBS OR ANDROID)
    add_library(FLAC SHARED ${FLAC_SRCS} ${extra_SRCS})
    target_link_libraries(FLAC ${FLAC_link_libraries})
    set_target_properties (FLAC PROPERTIES
        VERSION "${FLAC_LIB_VERSION}"
        SOVERSION "${FLAC_LIB_SOVERSION}"
        OUTPUT_NAME "FLAC"
    )
    set(FLAC_TGTS FLAC)
else()
    add_library(FLAC STATIC ${FLAC_SRCS} ${extra_SRCS})
    target_link_libraries(FLAC ${FLAC_link_libraries})
    set_target_properties (FLAC PROPERTIES
        OUTPUT_NAME "FLAC"
    )
    set(FLAC_TGTS FLAC)
endif()

set(FLAC_LIBRARIES
    FLAC ${FLAC_link_libraries}
)
set(FLAC_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(
    FLACXX_SRCS
    src/libFLAC++/metadata.cpp
    src/libFLAC++/stream_decoder.cpp
    src/libFLAC++/stream_encoder.cpp
)

if(BUILD_SHARED_LIBS OR ANDROID)
    add_library(FLACXX SHARED ${FLACXX_SRCS})
    add_dependencies(FLACXX ${FLAC_TGTS})
    target_link_libraries(FLACXX ${FLAC_LIBRARIES})
    set_target_properties (FLACXX PROPERTIES
        VERSION "${FLACXX_LIB_VERSION}"
        SOVERSION "${FLACXX_LIB_SOVERSION}"
        OUTPUT_NAME "FLAC++"
    )
    set(FLACXX_TGTS FLACXX)
else()
    add_library(FLACXX STATIC ${FLACXX_SRCS})
    add_dependencies(FLACXX ${FLAC_TGTS})
    target_link_libraries(FLACXX ${FLAC_LIBRARIES})
    set_target_properties (FLACXX PROPERTIES
        OUTPUT_NAME "FLAC++"
    )
    set(FLACXX_TGTS FLACXX)
endif()

set(FLACXX_LIBRARIES
    FLACXX ${FLAC_LIBRARIES}
)
set(FLACXX_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

export(PACKAGE FLAC)
export(PACKAGE FLACXX)

if(NOT MSVC)
###############################################################################
# install targets
# Create pkg-conf file.
# We use the same files as ./configure does, so we have to defined the same
# variables used there.

if(NOT SKIP_INSTALL_ALL)

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

    if (NOT DEFINED CMAKE_INSTALL_BINDIR)
        set (CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
    endif ()
    if (NOT DEFINED CMAKE_INSTALL_FULL_LIBDIR)
        set (CMAKE_INSTALL_FULL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
    endif ()
    if (NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
        set (CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
    endif ()
    set (prefix      "${CMAKE_INSTALL_PREFIX}")
    set (exec_prefix "${CMAKE_INSTALL_PREFIX}")
    set (includedir  "${CMAKE_INSTALL_INCLUDEDIR}")
    set (libdir      "${CMAKE_INSTALL_FULL_LIBDIR}")

    ###########################################################################
    # Add FLAC targets to the build-tree export set

    export(TARGETS ${FLAC_TGTS} FILE "${CMAKE_CURRENT_BINARY_DIR}/FLACTargets.cmake")

    configure_file (cmake/FLAC.pc.in FLAC.pc @ONLY)
    configure_file (cmake/FLACConfig.cmake.in FLACConfig.cmake @ONLY)
    configure_file (cmake/FLACConfigVersion.cmake.in FLACConfigVersion.cmake @ONLY)

    install (TARGETS ${FLAC_TGTS}
        EXPORT FLACTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

    install (FILES
        "${CMAKE_CURRENT_BINARY_DIR}/FLAC.pc"
        DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

    # Install the FLACConfig.cmake and FLACConfigVersion.cmake
    set(INSTALL_CMAKE_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/cmake/FLAC")
    install (FILES
        "${CMAKE_CURRENT_BINARY_DIR}/FLACConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/FLACConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

    # Install the export set for use with the install-tree
    install(EXPORT FLACTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

    file (GLOB FLAC_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/FLAC/*.h)
    install (FILES ${FLAC_HEADER_FILES}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/FLAC/)

    ###########################################################################
    # Add FLACXX targets to the build-tree export set

    export(TARGETS ${FLACXX_TGTS} FILE "${CMAKE_CURRENT_BINARY_DIR}/FLACXXTargets.cmake")

    configure_file (cmake/FLAC++.pc.in FLAC++.pc @ONLY)
    configure_file (cmake/FLACXXConfig.cmake.in FLACXXConfig.cmake @ONLY)
    configure_file (cmake/FLACXXConfigVersion.cmake.in FLACXXConfigVersion.cmake @ONLY)

    install (TARGETS ${FLACXX_TGTS}
        EXPORT FLACXXTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

    install (FILES
        "${CMAKE_CURRENT_BINARY_DIR}/FLAC++.pc"
        DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

    # Install the FLACXXConfig.cmake and FLAC++ConfigVersion.cmake
    set(INSTALL_CMAKE_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/cmake/FLACXX")
    install (FILES
        "${CMAKE_CURRENT_BINARY_DIR}/FLACXXConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/FLACXXConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

    # Install the export set for use with the install-tree
    install(EXPORT FLACXXTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

    file (GLOB FLACXX_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/FLAC++/*.h)
    install(FILES ${FLACXX_HEADER_FILES}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/FLAC++/)

endif()
endif()
