add_library(woop_core STATIC 
  wad.cpp
  level.cpp
  bsp.cpp
  window.cpp
  camera.cpp
  display_rect.cpp
  renderer.cpp
  shader.cpp
  player.cpp
)

target_include_directories(woop_core PUBLIC
  "${CMAKE_SOURCE_DIR}/include/"
)

target_link_libraries(woop_core PUBLIC
  glad
  glfw
  glm::glm
)

# Compiler warning levels
target_compile_options(woop_core PRIVATE 
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Werror -Wextra -Wconversion -Wsign-conversion -pedantic-errors>
  $<$<CXX_COMPILER_ID:MSVC>:
    /WX /W4>
)

# Logging
target_compile_definitions(woop_core PUBLIC
  $<$<BOOL:${WOOP_ENABLE_LOGGING}>:"WOOP_ENABLE_LOGGING">
  $<$<BOOL:${WOOP_COLOR_LOGGING}>:"WOOP_COLOR_LOGGING">
)