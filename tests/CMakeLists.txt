include(FetchGTest.cmake)
enable_testing()

add_executable(woop_tests
  placeholder.cpp
)

target_link_libraries(woop_tests PRIVATE 
  woop_core
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(woop_tests)
