add_subdirectory(core)

add_executable(woop
  main.cpp
)

target_link_libraries(woop PRIVATE
  woop_core
)