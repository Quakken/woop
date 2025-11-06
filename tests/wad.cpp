/**
 * @file wad.cpp
 * @authors quak
 * @brief Tests for Wad parsing and navigation.
 * @note These tests require an official DOOM wad to run.
 */

#include "gtest/gtest.h"
#include "log.hpp"
#include "wad.hpp"

// Path to the wad that will be used for testing.
constexpr const char* wad_path = "wads/doom1.wad";
// Path to an invalid wad file.
constexpr const char* invalid_path = "path/to/an/invalid/wad";

TEST(Wads, Open) {
  // Opening a wad with "open"
  {
    woop::Wad wad;
    wad.open(wad_path);
    EXPECT_TRUE(wad.is_open());
  }
  // Opening a wad via constructor
  {
    woop::Wad wad(wad_path);
    EXPECT_TRUE(wad.is_open());
  }
  // Opening a wad that already contains data
  {
    woop::Wad wad(wad_path);
    wad.open(wad_path);
    EXPECT_TRUE(wad.is_open());
  }
  // Opening a closed wad
  {
    woop::Wad wad(wad_path);
    wad.close();
    wad.open(wad_path);
    EXPECT_TRUE(wad.is_open());
  }
  // Opening a wad that doesn't exist
  {
    EXPECT_THROW(woop::Wad{invalid_path}, woop::WadException);
  }
}

TEST(Wads, Close) {
  // Closing a wad
  {
    woop::Wad wad(wad_path);
    wad.close();
    EXPECT_FALSE(wad.is_open());
  }
  // Closing a wad that hasn't been opened
  {
    woop::Wad wad;
    wad.close();
    EXPECT_FALSE(wad.is_open());
  }
}

TEST(Wads, GetLump) {
  woop::Wad wad(wad_path);
  // Retrieving a lump with data
  {
    woop::Lump lump = wad.get_lump("PLAYPAL");
    EXPECT_STREQ(lump.name.c_str(), "PLAYPAL");
    EXPECT_GT(lump.data.size(), 0);
  }
  // Retrieving a virtual lump
  {
    woop::Lump lump = wad.get_lump("E1M1");
    EXPECT_STREQ(lump.name.c_str(), "E1M1");
    EXPECT_EQ(lump.data.size(), 0);
  }
  // Retrieving sequential lumps
  {
    woop::Lump e1m1_things = wad.get_lump("E1M1", "THINGS");
    woop::Lump e1m2_things = wad.get_lump("E1M2", "THINGS");
    EXPECT_STREQ(e1m1_things.name.c_str(), e1m2_things.name.c_str());
    EXPECT_NE(e1m1_things.data.size(), e1m2_things.data.size());
  }
  // Retrieving a lump that doesn't exist
  EXPECT_THROW(wad.get_lump("INVALIDLUMP"), woop::WadException);
}

TEST(Wads, Iterator) {
  woop::Wad wad(wad_path);
  // Search for the start and end of the sprites block (very inefficient)
  bool start_found, end_found;
  for (const auto& it : wad) {
    if (it.name == "S_START")
      start_found = true;
    if (it.name == "S_END")
      end_found = true;
    if (start_found && end_found)
      break;
  }
  EXPECT_TRUE(start_found && end_found);
}