include(FetchGTest.cmake)
enable_testing()

add_executable(woop_tests
  wad.cpp
)

target_link_libraries(woop_tests PRIVATE 
  woop_core
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(woop_tests)

# Copy wads (required by some tests)
file(
  COPY "${CMAKE_CURRENT_SOURCE_DIR}/wads" 
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
)
