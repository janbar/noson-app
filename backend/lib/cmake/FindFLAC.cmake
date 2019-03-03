#
# Find and configure FLAC package
#
if(FLAC_INCLUDE_DIR AND FLAC_LIBRARIES)
  set(FLAC_FIND_QUIETLY true)
endif()

find_path(FLAC_INCLUDE_DIR FLAC/metadata.h)
find_library(FLAC_LIBRARIES FLAC)

if(FLAC_INCLUDE_DIR AND FLAC_LIBRARIES)
   set(FLAC_FOUND true)
   if(NOT FLAC_FIND_QUIETLY)
     message(STATUS "Found Flac: ${FLAC_LIBRARIES}")
   endif()
else()
   if(FLAC_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find FLAC")
   endif()
   if(NOT FLAC_FIND_QUIETLY)
      message(STATUS "Could not find FLAC")
   endif()
endif()

mark_as_advanced(FLAC_INCLUDE_DIR FLAC_LIBRARIES)

