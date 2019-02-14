#
# Find and configure pulse-simple package
#
if(PULSESIMPLE_INCLUDE_DIR AND PULSESIMPLE_LIBRARIES)
  set(PULSESIMPLE_FIND_QUIETLY true)
endif()

find_path(PULSESIMPLE_INCLUDE_DIR pulse/simple.h)
find_library(PULSESIMPLE_LIBRARIES pulse-simple)

if(PULSESIMPLE_INCLUDE_DIR AND PULSESIMPLE_LIBRARIES)
   set(PULSESIMPLE_FOUND true)
   if(NOT PULSESIMPLE_FIND_QUIETLY)
     message(STATUS "Found Pulse-simple: ${PULSESIMPLE_LIBRARIES}")
   endif()
else()
   if(PULSESIMPLE_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Pulse-simple")
   endif()
   if(NOT PULSESIMPLE_FIND_QUIETLY)
      message(STATUS "Could not find Pulse-simple")
   endif()
endif()

mark_as_advanced(PULSESIMPLE_INCLUDE_DIR PULSESIMPLE_LIBRARIES)
