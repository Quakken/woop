#include "level.hpp"
#include "glm/fwd.hpp"
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

TEST(Levels, BSP) {
  // Traversing BSP should always terminate in a subsector.
  {
    woop::Level level(wad, "E1M1");
    woop::Node* node = &level.get_root_node();
    while (node->is_node_left())
      node = &node->get_node_left();
  }
  // You should always be able to find the subsector closest to a point
  {
    woop::Level level(wad, "E1M1");
    woop::Node* node = &level.get_root_node();
    woop::Subsector* subsector;
    glm::vec2 point = {0, 0};
    while (true) {
      woop::Node::Child child = node->get_nearest_child(point);
      if (node->is_node(child))
        node = &node->get_node(child);
      else {
        subsector = &node->get_subsector(child);
        break;
      }
    }
  }
}