cmake_minimum_required (VERSION 2.8.9 FATAL_ERROR)
cmake_policy (VERSION 2.8)

project (noson)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#include (GNUInstallDirs)

# Options
OPTION (BUILD_SHARED_LIBS "Build shared libraries." OFF)
if (MSVC)
  # This option must match the settings used in your program, in particular if you
  # are linking statically
  OPTION (STATIC_CRT "Link the static CRT libraries" OFF)
endif ()

###############################################################################
# set lib version here
set (noson_VERSION_MAJOR 1)
set (noson_VERSION_MINOR 12)
set (noson_VERSION_PATCH 3)

set (noson_VERSION ${noson_VERSION_MAJOR}.${noson_VERSION_MINOR}.${noson_VERSION_PATCH})
set (NOSON_LIB_VERSION ${noson_VERSION})
set (NOSON_LIB_SOVERSION ${noson_VERSION_MAJOR})

###############################################################################
# add definitions
if (MSVC)
  add_definitions ("/D_CRT_SECURE_NO_WARNINGS")
  if (STATIC_CRT)
    set (CMAKE_C_FLAGS_RELEASE "/MT")
    set (CMAKE_C_FLAGS_DEBUG "/MTd")
    set (CMAKE_CXX_FLAGS_RELEASE "/MT")
    set (CMAKE_CXX_FLAGS_DEBUG "/MTd")
  endif ()
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /W3")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W3")
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /Od /RTC1 /EHsc /nologo")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /Od /RTC1 /EHsc /nologo")
endif ()

if (NOT MSVC)
  if (NOT CYGWIN)
    add_definitions ("-fPIC")
  endif ()

  add_definitions ("-Wall")

  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  else()
    message(FATAL_ERROR "CXX compiler doesn't support standard c++11")
  endif ()
endif ()

###############################################################################
# configure
include (CheckFunctionExists)
include (CheckFunctionKeywords)
find_package (Threads REQUIRED)

check_function_exists (timegm CHK_TIMEGM)
if (CHK_TIMEGM)
    set (HAVE_TIMEGM 1)
else ()
    set (HAVE_TIMEGM 0)
endif ()

check_function_exists (localtime_r CHK_LOCALTIME_R)
if (CHK_LOCALTIME_R)
    set (HAVE_LOCALTIME_R 1)
else ()
    set (HAVE_LOCALTIME_R 0)
endif ()

check_function_exists (gmtime_r CHK_GMTIME_R)
if (CHK_GMTIME_R)
    set (HAVE_GMTIME_R 1)
else ()
    set (HAVE_GMTIME_R 0)
endif ()

if (NOT ZLIB_FOUND)
    find_package (ZLIB REQUIRED)
endif()
if (ZLIB_FOUND)
    include_directories (${ZLIB_INCLUDE_DIRS})
    set (HAVE_ZLIB 1)
else ()
    set (HAVE_ZLIB 0)
endif ()

if (NOT OPENSSL_FOUND)
    find_package(OpenSSL REQUIRED)
endif()
if (OPENSSL_FOUND)
    include_directories (${OPENSSL_INCLUDE_DIR})
    set (HAVE_OPENSSL 1)
else ()
    set (HAVE_OPENSSL 0)
endif ()

# Check what the inline keyword is.
check_function_keywords ("inline")
check_function_keywords ("__inline")
check_function_keywords ("__inline__")
if (HAVE_INLINE)
   set (CC_INLINE inline)
elseif (HAVE___INLINE)
   set (CC_INLINE __inline)
elseif (HAVE___INLINE__)
   set (CC_INLINE __inline__)
else ()
   # no inline on this platform
   set (CC_INLINE)
endif ()

# About streamer
if (NOT FLAC_FOUND)
    find_package(FLAC)
endif ()
if (NOT FLACXX_FOUND)
    find_package(FLACXX)
endif ()
find_package(Pulse)
find_package(Pulse-simple)

if (FLACXX_FOUND AND FLAC_FOUND)
  set (HAVE_FLAC 1 CACHE INTERNAL "Enable FLAC feature" FORCE)
  if (PULSESIMPLE_FOUND AND PULSE_FOUND)
    set (HAVE_PULSEAUDIO 1 CACHE INTERNAL "Enable PulseAudio feature" FORCE)
  endif ()
endif ()

set (noson_PUBLIC_DIR "${CMAKE_CURRENT_BINARY_DIR}/public/noson")

# configure the public config file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/src/local_config.h.in
                 ${noson_PUBLIC_DIR}/local_config.h)

# Copy the header files to the public folder
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/locked.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/intrinsic.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sharedptr.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/avtransport.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/contentdirectory.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/deviceproperties.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/didlparser.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalitem.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/element.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/eventhandler.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/renderingcontrol.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/service.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/subscription.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sonosplayer.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sonossystem.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sonostypes.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sonoszone.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/zonegrouptopology.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/musicservices.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/smapi.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/smapimetadata.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/smaccount.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/alarmclock.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/alarm.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/requestbroker.h
          DESTINATION ${noson_PUBLIC_DIR})

# Copy headers for streams
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/audioencoder.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/audioformat.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/audiosource.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/audiostream.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/iodevice.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/streambuffer.h
          DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/streamreader.h
        DESTINATION ${noson_PUBLIC_DIR})
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/filestreamer.h
        DESTINATION ${noson_PUBLIC_DIR})
if(HAVE_FLAC)
  file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/flacencoder.h
            DESTINATION ${noson_PUBLIC_DIR})
  if(HAVE_PULSEAUDIO)
    file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/pacontrol.h
              DESTINATION ${noson_PUBLIC_DIR})
    file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/pasource.h
              DESTINATION ${noson_PUBLIC_DIR})
    file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/pulsestreamer.h
              DESTINATION ${noson_PUBLIC_DIR})
  endif()
endif()
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/imageservice.h
          DESTINATION ${noson_PUBLIC_DIR})

file (GLOB PUBLIC_HEADER_FILES ${noson_PUBLIC_DIR}/*.h)

include_directories (${noson_PUBLIC_DIR})

###############################################################################
# add sources
file (GLOB SRC_FILES
  src/private/*.c
  src/private/*.cpp
  src/private/*.h
)

list (APPEND SRC_FILES
  src/data/datareader.cpp
  src/data/datareader.h
  src/alarmclock.cpp
  src/alarmclock.h
  src/alarm.cpp
  src/alarm.h
  src/audioencoder.cpp
  src/audioencoder.h
  src/audioformat.h
  src/audiosource.cpp
  src/audiosource.h
  src/audiostream.h
  src/avtransport.cpp
  src/avtransport.h
  src/contentdirectory.cpp
  src/contentdirectory.h
  src/deviceproperties.cpp
  src/deviceproperties.h
  src/didlparser.cpp
  src/didlparser.h
  src/digitalitem.cpp
  src/digitalitem.h
  src/element.h
  src/eventhandler.cpp
  src/eventhandler.h
  src/filepicreader.cpp
  src/filepicreader.h
  src/filestreamer.cpp
  src/filestreamer.h
  src/imageservice.cpp
  src/imageservice.h
  src/intrinsic.cpp
  src/intrinsic.h
  src/iodevice.cpp
  src/iodevice.h
  src/locked.cpp
  src/locked.h
  src/musicservices.cpp
  src/musicservices.h
  src/renderingcontrol.cpp
  src/renderingcontrol.h
  src/requestbroker.cpp
  src/requestbroker.h
  src/service.cpp
  src/service.h
  src/sharedptr.h
  src/smaccount.cpp
  src/smaccount.h
  src/smapi.cpp
  src/smapi.h
  src/smapimetadata.cpp
  src/smapimetadata.h
  src/sonosplayer.cpp
  src/sonosplayer.h
  src/sonossystem.cpp
  src/sonossystem.h
  src/sonostypes.cpp
  src/sonostypes.h
  src/sonoszone.cpp
  src/sonoszone.h
  src/streambuffer.cpp
  src/streambuffer.h
  src/streamreader.h
  src/subscription.cpp
  src/subscription.h
  src/zonegrouptopology.cpp
  src/zonegrouptopology.h
)

if (HAVE_FLAC)
  include_directories (${FLACXX_INCLUDE_DIR})
  list (APPEND STREAM_SRC_FILES
    src/flacencoder.cpp
    src/flacencoder.h
  )
  if (HAVE_PULSEAUDIO)
    include_directories (${PULSESIMPLE_INCLUDE_DIR} ${PULSE_INCLUDE_DIR})
    list (APPEND STREAM_SRC_FILES
      src/pacontrol.cpp
      src/pacontrol.h
      src/pasource.cpp
      src/pasource.h
      src/pulsestreamer.cpp
      src/pulsestreamer.h
    )
  endif ()
endif ()

set (OS_SRC_FILES src/private/os/threads/threadpool.cpp)

if (MSVC)
  list (APPEND OS_SRC_FILES src/private/os/windows/winpthreads.c)
endif ()

set (NOSON_SOURCES ${SRC_FILES} ${OS_SRC_FILES} ${STREAM_SRC_FILES})

###############################################################################
# add targets
set (noson_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
if (ZLIB_FOUND)
  list (APPEND noson_LIBRARIES ${ZLIB_LIBRARIES})
endif ()
if (OPENSSL_FOUND)
  list (APPEND noson_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
endif ()
if(HAVE_FLAC)
  list(APPEND noson_LIBRARIES ${FLACXX_LIBRARIES} ${FLAC_LIBRARIES})
endif()
if(HAVE_PULSEAUDIO)
  list(APPEND noson_LIBRARIES ${PULSESIMPLE_LIBRARIES} ${PULSE_LIBRARIES})
endif()

if (MSVC)
  list (APPEND noson_LIBRARIES ws2_32)
else ()
  list (APPEND noson_LIBRARIES m)
  find_library (LIBRT rt)
  if (LIBRT)
    list (APPEND noson_LIBRARIES rt)
  endif ()
endif ()

if (BUILD_SHARED_LIBS)
  add_library (noson SHARED ${NOSON_SOURCES})
else ()
  add_library (noson STATIC ${NOSON_SOURCES})
endif ()
target_link_libraries (noson ${noson_LIBRARIES})
set_target_properties (noson PROPERTIES
    VERSION "${NOSON_LIB_VERSION}"
    SOVERSION "${NOSON_LIB_SOVERSION}")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE noson)

###############################################################################
# install targets
# Create pkg-conf file.
# (We use the same files as ./configure does, so we
#  have to defined the same variables used there).

if (NOT SKIP_INSTALL_ALL)
  # Add all targets to the build-tree export set
  export(TARGETS noson FILE "${CMAKE_CURRENT_BINARY_DIR}/nosonTargets.cmake")

  if (NOT MSVC)
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
    set (includedir  "${CMAKE_INSTALL_INCLUDEDIR}/noson")
    set (libdir      "${CMAKE_INSTALL_FULL_LIBDIR}")
    configure_file (noson.pc.in noson.pc @ONLY)
    configure_file (nosonConfig.cmake.in nosonConfig.cmake @ONLY)
    configure_file (nosonConfigVersion.cmake.in nosonConfigVersion.cmake @ONLY)

    install (TARGETS noson
      EXPORT nosonTargets
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

    install (FILES
      "${CMAKE_CURRENT_BINARY_DIR}/noson.pc"
      DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

    # Install the nosonConfig.cmake and nosonConfigVersion.cmake
    set(INSTALL_CMAKE_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/cmake/noson")
    install (FILES
      "${CMAKE_CURRENT_BINARY_DIR}/nosonConfig.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/nosonConfigVersion.cmake"
      DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

    # Install the export set for use with the install-tree
    install(EXPORT nosonTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

  endif ()

  install (FILES ${PUBLIC_HEADER_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/noson/)
endif ()
