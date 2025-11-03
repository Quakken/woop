cmake_minimum_required(VERSION 3.40)
project(woop)

set(CMAKE_CXX_STANDARD_REQUIRED TRUE FORCE)
set(CMAKE_CXX_STANDARD 17 FORCE)

add_subdirectory(thirdparty)
add_subdirectory(src)