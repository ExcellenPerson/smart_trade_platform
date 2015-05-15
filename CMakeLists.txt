cmake_minimum_required (VERSION 2.8)

project (SmartTradePlatform)

# The version number.
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)


# does this system provide the log and exp functions?
include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
check_function_exists (signalfd HAVE_SIGNALFD)
check_function_exists (signaltest HAVE_SIGNALFDtest)
 
# should we use our own math functions
option(USE_SMART_UTILS_LIB "Use SmartUtils library" ON)
 
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/SmartUtils/Base/Config.h.in"
  "${PROJECT_SOURCE_DIR}/SmartUtils/Base/Config.h"
  )


# add the MathFunctions library?
if (USE_SMART_UTILS_LIB)
  include_directories ("${PROJECT_SOURCE_DIR}/SmartUtils")
  add_subdirectory (SmartUtils)
  set (EXTRA_LIBS ${EXTRA_LIBS} SmartUtils)
endif (USE_SMART_UTILS_LIB)
 
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
#include_directories("${PROJECT_BINARY_DIR}")
 
# add the executable
add_subdirectory (STPTest)



