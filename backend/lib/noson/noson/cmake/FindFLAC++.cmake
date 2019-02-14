#
# Find and configure FLAC++ package
#
if(FLAC++_INCLUDE_DIR AND FLAC++_LIBRARIES)
  set(FLAC++_FIND_QUIETLY true)
endif()

find_path(FLAC++_INCLUDE_DIR FLAC++/metadata.h)
find_library(FLAC++_LIBRARIES FLAC++)

if(FLAC++_INCLUDE_DIR AND FLAC++_LIBRARIES)
   set(FLAC++_FOUND true)
   if(NOT FLAC++_FIND_QUIETLY)
     message(STATUS "Found Flac++: ${FLAC++_LIBRARIES}")
   endif()
else()
   if(FLAC++_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find FLAC++")
   endif()
   if(NOT FLAC++_FIND_QUIETLY)
      message(STATUS "Could not find FLAC++")
   endif()
endif()

mark_as_advanced(FLAC++_INCLUDE_DIR FLAC++_LIBRARIES)
