#include "level.hpp"
#include "gtest/gtest.h"

constexpr const char* wad_path = "wads/doom1.wad";

const woop::Wad wad(wad_path);

TEST(Levels, Open) {
  // Constructor
  {
    woop::Level level(wad, "E1M1");
  }
  // level::Open
  {
    woop::Level level;
    level.open(wad, "E1M1");
    level.open(wad, "E1M2");
  }
  // Copy constructor etc.
  {
    woop::Level level1(wad, "E1M1");
    woop::Level level2 = level1;
    woop::Level level3(level2);
  }
}

TEST(Levels, Close) {
  // Closing level normally
  {
    woop::Level level(wad, "E1M1");
    EXPECT_TRUE(level.is_open());
    level.close();
    EXPECT_FALSE(level.is_open());
  }
  // Closing level that is not open
  {
    woop::Level level;
    level.close();
    EXPECT_FALSE(level.is_open());
  }
}