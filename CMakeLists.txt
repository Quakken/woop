cmake_minimum_required(VERSION 3.40)
project(woop)

set(CMAKE_CXX_STANDARD_REQUIRED TRUE FORCE)
set(CMAKE_CXX_STANDARD 17 FORCE)

set(WOOP_ENABLE_TESTING TRUE CACHE BOOL "Whether to build tests")
set(WOOP_ENABLE_LOGGING TRUE CACHE BOOL "Whether to log messages to standard output streams")
set(WOOP_COLOR_LOGGING TRUE CACHE BOOL "Whether to enable colorful log messages")

add_subdirectory(thirdparty)
add_subdirectory(src)
if (${WOOP_ENABLE_TESTING})
  add_subdirectory(tests)
endif()