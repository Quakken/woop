add_subdirectory(core)

add_executable(woop
  main.cpp
)

target_link_libraries(woop PRIVATE
  woop_core
)

# Copy assets
file(
  COPY "${CMAKE_SOURCE_DIR}/assets" 
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
)