#
# Find and configure pulse package
#
if(PULSE_INCLUDE_DIR AND PULSE_LIBRARIES)
  set(PULSE_FIND_QUIETLY true)
endif()

find_path(PULSE_INCLUDE_DIR pulse/pulseaudio.h)
find_library(PULSE_LIBRARIES pulse)

if(PULSE_INCLUDE_DIR AND PULSE_LIBRARIES)
   set(PULSE_FOUND true)
   if(NOT PULSE_FIND_QUIETLY)
     message(STATUS "Found Pulse: ${PULSE_LIBRARIES}")
   endif()
else()
   if(PULSE_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Pulse")
   endif()
   if(NOT PULSE_FIND_QUIETLY)
      message(STATUS "Could not find Pulse")
   endif()
endif()

mark_as_advanced(PULSE_INCLUDE_DIR PULSE_LIBRARIES)
