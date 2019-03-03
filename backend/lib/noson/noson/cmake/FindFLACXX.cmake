#
# Find and configure FLACXX package
#
if(FLACXX_INCLUDE_DIR AND FLACXX_LIBRARIES)
  set(FLACXX_FIND_QUIETLY true)
endif()

find_path(FLACXX_INCLUDE_DIR FLAC++/metadata.h)
find_library(FLACXX_LIBRARIES FLAC++)

if(FLACXX_INCLUDE_DIR AND FLACXX_LIBRARIES)
   set(FLACXX_FOUND true)
   if(NOT FLACXX_FIND_QUIETLY)
     message(STATUS "Found Flac++: ${FLACXX_LIBRARIES}")
   endif()
else()
   if(FLACXX_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find FLAC++")
   endif()
   if(NOT FLACXX_FIND_QUIETLY)
      message(STATUS "Could not find FLAC++")
   endif()
endif()

mark_as_advanced(FLACXX_INCLUDE_DIR FLACXX_LIBRARIES)
