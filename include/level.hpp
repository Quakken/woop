/**
 * @file level.hpp
 * @authors quak
 * @brief Declares the Level class, which stores information needed to render
 * and navigate a single level in the engine.
 */

#pragma once

#include "exception.hpp" /* woop::Exception */
#include "wad.hpp"       /* woop::Wad, woop::Lump */
#include "bsp.hpp"       /* woop::Node */
#include "glm/vec2.hpp"  /* glm::vec2 */
#include <vector>        /* std::vector */

namespace woop {
struct Linedef;

/**
 * @brief Exception thrown when a level encounters an error.
 */
class LevelException : public Exception {
 public:
  /**
   * @brief Details the cause of the exception.
   */
  enum class Type {
    InvalidData,
  };

  LevelException(Type type, const std::string_view& what = "LevelException")
      : Exception(what), t(type) {}

  /**
   * @brief Returns the type of exception that was thrown
   */
  Type type() const noexcept { return t; }

 private:
  Type t;
};

/**
 * LEVEL DATA
 *
 * These types hold information about the structure of a level. The wiki does a
 * great job of explaining the purpose of each type, and I would definitely
 * recommend looking through it if any of this seems confusing.
 * https://doomwiki.org/wiki/Map_format
 */

/**
 * @brief Stores data about a single map sector
 */
struct Sector {
  struct {
    int16_t height;
    std::string texture;
    /* TODO: Texture& texture; */
  } floor;
  struct {
    int16_t height;
    std::string texture;
    /* TODO: Texture& texture; */
  } ceiling;
  int16_t light_level;
  std::vector<Linedef*> lines;
  /* TODO: Specials and tags */
};

/**
 * @brief Stores information about one side of a linedef (wall)
 */
struct Sidedef {
  /*
  TODO: When textures are added...
  Texture& upper;
  Texture& lower;
  Texture& middle;
  */
  std::string upper_name;
  std::string lower_name;
  std::string middle_name;
  Sector& sector_facing;
  glm::vec2 offset;
};

/**
 * @brief Stores information about a line (wall) in a map
 */
struct Linedef {
  glm::vec2& start;
  glm::vec2& end;
  Sidedef* front;
  Sidedef* back;
  /* TODO: Flags and specials */
};

/**
 * @brief Stores information about a segment of a linedef (wall)
 */
struct Seg {
  glm::vec2& start;
  glm::vec2& end;
  Linedef& linedef;
  Sidedef* sidedef;
  float angle;
  int16_t offset;
};

/**
 * @brief Stores information about a subsector, a sector whose segments make up
 * a convex polygon.
 */
struct Subsector {
  std::vector<Seg*> segs;
};

/**
 * @brief Stores all information about a level
 */
class Level {
 public:
  Level();
  /**
   * @brief Opens a level with the given name from a wad.
   */
  Level(const Wad& wad, const std::string& name);

  /**
   * @brief Opens a level with the given name from a wad.
   */
  void open(const Wad& wad, const std::string& name);
  /**
   * @brief Closes the level, releasing all data.
   */
  void close();

  bool is_open() const noexcept { return loaded; }

  std::string get_name() const noexcept { return name; }

  const Node& get_root_node() const;
  Node& get_root_node();

 private:
  /**
   * RAW TYPES
   *
   * All types used to extract map information from a lump. These only serve as
   * an intermediary representation, and aren't used outside of level loading.
   */
  struct RawThing {
    int16_t x_pos;
    int16_t y_pos;
    int16_t angle;
    int16_t type;
    int16_t flags;
  };
  struct RawLinedef {
    int16_t start_vertex;
    int16_t end_vertex;
    int16_t flags;
    int16_t special;
    int16_t tag;
    int16_t front_sidedef;
    int16_t back_sidedef;
  };
  struct RawSidedef {
    int16_t x_offset;
    int16_t y_offset;
    char upper_name[8];
    char lower_name[8];
    char middle_name[8];
    int16_t sector_facing;
  };
  struct RawVertex {
    int16_t x_pos;
    int16_t y_pos;
  };
  struct RawSeg {
    int16_t start_vertex;
    int16_t end_vertex;
    int16_t angle;
    int16_t linedef;
    int16_t direction; /* 0: front of linedef, 1: back of linedef */
    int16_t offset;
  };
  struct RawSubsector {
    int16_t seg_count;
    int16_t first_seg;
  };
  struct RawNode {
    int16_t x_part_start;
    int16_t y_part_start;
    int16_t x_part_delta;
    int16_t y_part_delta;
    union {
      int16_t data[4];
      struct {
        int16_t top;
        int16_t bottom;
        int16_t left;
        int16_t right;
      } names;
    } right_bounds;
    union {
      int16_t data[4];
      struct {
        int16_t top;
        int16_t bottom;
        int16_t left;
        int16_t right;
      } names;
    } left_bounds;
    /* Sign bit determines child type (0: subnode, 1: subsector) */
    int16_t right_child;
    int16_t left_child;
  };
  struct RawSector {
    int16_t floor_height;
    int16_t ceiling_height;
    char floor_texture[8];
    char ceiling_texture[8];
    int16_t light_level;
    int16_t special;
    int16_t tag;
  };

 private:
  void populate_level_data(const Wad& wad);
  void populate_sectors(const Wad& wad);
  void populate_subsectors(const Wad& wad);
  void populate_segs(const Wad& wad);
  void populate_linedefs(const Wad& wad);
  void populate_sidedefs(const Wad& wad);
  void populate_vertices(const Wad& wad);
  void populate_nodes(const Wad& wad);
  /**
   * @brief Fills all data that could not be assigned during population (due to
   * dependency circles)
   */
  void finish_connections();

  std::vector<Sector> sectors;
  std::vector<Subsector> subsectors;
  std::vector<Seg> segs;
  std::vector<Linedef> linedefs;
  std::vector<Sidedef> sidedefs;
  std::vector<glm::vec2> vertices;
  std::vector<Node> nodes;
  /* TODO: Reject and blockmap */
  bool loaded;
  std::string name;
  Node* bsp_root;
};
}  // namespace woop