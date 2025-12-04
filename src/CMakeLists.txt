add_subdirectory(core)

add_executable(woop
  main.cpp
)

target_link_libraries(woop PRIVATE
  woop_core
)

# Copy assets
add_custom_command(
        TARGET woop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/assets
                ${CMAKE_CURRENT_BINARY_DIR}/assets)