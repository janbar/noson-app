cmake_minimum_required (VERSION 2.6 FATAL_ERROR)
cmake_policy (VERSION 2.6)
set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required

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

if (NOT WIN32)
  OPTION (REQUIRE_CXX_98 "Require standard c++98" OFF)
endif ()

###############################################################################
# set lib version here
set (NOSON_LIB_VERSION "1.0.0")
set (NOSON_LIB_SOVERSION "1")

###############################################################################
# add definitions
if (MSVC)
  add_definitions ("/D_USE_32BIT_TIME_T /D_CRT_SECURE_NO_WARNINGS")
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

if (NOT WIN32 AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX))
  if (NOT CYGWIN)
    add_definitions ("-fPIC")
  endif ()
  add_definitions ("-Wall")
  include (CheckLibraryExists)
  include (CheckAtomic)
  if (HAS_BUILTIN_SYNC_ADD_AND_FETCH)
    add_definitions ("-DHAS_BUILTIN_SYNC_ADD_AND_FETCH")
  endif ()
  if (HAS_BUILTIN_SYNC_SUB_AND_FETCH)
    add_definitions ("-DHAS_BUILTIN_SYNC_SUB_AND_FETCH")
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

if (REQUIRE_CXX_98)
  set (CXX_STANDARD "199711L")
  message(STATUS "c++98 activated")
else ()
  if (MSVC)
    set (CXX_STANDARD "201103L")
  else ()
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    if(COMPILER_SUPPORTS_CXX11)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
      set (CXX_STANDARD "201103L")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
      set (CXX_STANDARD "199711L")
    endif ()
  endif ()
endif ()

# configure the public config file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/src/local_config.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/include/local_config.h)

# Copy the header files to the public include folder

file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/locked.h
           DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/intrinsic.h
           DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sharedptr.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/avtransport.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/contentdirectory.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/deviceproperties.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/digitalitem.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/element.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/eventhandler.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/renderingcontrol.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/service.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/subscription.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sonosplayer.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sonossystem.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sonostypes.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/sonoszone.h
          DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)

include_directories (
  ${CMAKE_CURRENT_BINARY_DIR}/include/.)

###############################################################################
# add sources
file (GLOB SRC_FILES
  src/private/*.c
  src/private/*.cpp
  src/*.cpp)

file (GLOB OS_SRC_FILES
  src/private/os/threads/threadpool.cpp)

if (MSVC)
  list (APPEND OS_SRC_FILES
    src/private/os/windows/winpthreads.c)
endif ()

set (NOSON_SOURCES
  ${SRC_FILES} ${OS_SRC_FILES})

###############################################################################
# add targets
set (noson_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
if (WIN32)
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

###############################################################################
# install targets
# Create pkg-conf file.
# (We use the same files as ./configure does, so we
#  have to defined the same variables used there).
if (NOT WIN32)
  if (NOT DEFINED CMAKE_INSTALL_BINDIR)
    set (CMAKE_INSTALL_BINDIR bin)
  endif ()
  if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set (CMAKE_INSTALL_LIBDIR lib)
  endif ()
  if (NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
    set (CMAKE_INSTALL_INCLUDEDIR include)
  endif ()
  set (prefix      ${CMAKE_INSTALL_PREFIX})
  set (exec_prefix ${CMAKE_INSTALL_PREFIX})
  set (libdir      ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
  set (VERSION     ${NOSON_LIB_VERSION})
  configure_file (noson.pc.in noson.pc @ONLY)

  install (TARGETS noson
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

  install (FILES
    ${CMAKE_CURRENT_BINARY_DIR}/noson.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif ()

install (FILES
  ${CMAKE_CURRENT_BINARY_DIR}/include/local_config.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/locked.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/intrinsic.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/sharedptr.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/avtransport.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/contentdirectory.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/deviceproperties.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/digitalitem.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/element.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/eventhandler.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/renderingcontrol.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/service.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/subscription.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/sonosplayer.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/sonossystem.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/sonostypes.h
  ${CMAKE_CURRENT_BINARY_DIR}/include/sonoszone.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/noson/)
