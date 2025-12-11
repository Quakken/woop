add_subdirectory(core)

add_executable(woop
  main.cpp
  config.cpp
)

target_link_libraries(woop PRIVATE
  woop_core
  tomlplusplus::tomlplusplus
)

# Copy assets
add_custom_command(
  TARGET woop POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets
          ${CMAKE_CURRENT_BINARY_DIR}/assets
)
# Copy configuration
add_custom_command(
  TARGET woop POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy 
          ${CMAKE_SOURCE_DIR}/config.toml
          ${CMAKE_CURRENT_BINARY_DIR}/config.toml
)