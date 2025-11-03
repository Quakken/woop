add_executable(woop
  test.cpp
)

target_include_directories(woop PRIVATE
  "${CMAKE_SOURCE_DIR}/include/"
)

target_link_libraries(woop PRIVATE
  glad
  glfw
  glm::glm
)

# Compiler warning levels
target_compile_options(woop PRIVATE 
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Werror -Wextra -Wconversion -Wsign-conversion -pedantic-errors>
  $<$<CXX_COMPILER_ID:MSVC>:
    /WX /W4>
)