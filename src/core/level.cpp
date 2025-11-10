/**
 * @file level.cpp
 * @authors quak
 * @brief Defines members of the Level class.
 */

#include "level.hpp"
#include "bsp.hpp"

namespace woop {
constexpr float doom_angle_to_deg(int16_t angle) {
  return static_cast<float>(angle) * 180.0f / 32767.0f;
}

Level::Level() : loaded(false) {}

Level::Level(const Wad& wad, const std::string& level_name) {
  open(wad, level_name);
}

void Level::open(const Wad& wad, const std::string& level_name) {
  close();
  name = level_name;
  populate_level_data(wad);
  loaded = true;
}

void Level::close() {
  if (!loaded)
    return;
  sectors.clear();
  subsectors.clear();
  segs.clear();
  linedefs.clear();
  sidedefs.clear();
  nodes.clear();
  loaded = false;
}

const Node& Level::get_root_node() const {
  if (!is_open())
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to access root node of unloaded level.");
  return *bsp_root;
}
Node& Level::get_root_node() {
  if (!is_open())
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to access root node of unloaded level.");
  return *bsp_root;
}

void Level::populate_level_data(const Wad& wad) {
  populate_vertices(wad);
  populate_sectors(wad);
  populate_sidedefs(wad);
  populate_linedefs(wad);
  populate_segs(wad);
  populate_subsectors(wad);
  populate_nodes(wad);
  finish_connections();
}
void Level::populate_sectors(const Wad& wad) {
  const Lump& lump = wad.get_lump(name, "SECTORS");
  std::vector<RawSector> raw_data = lump.get_data_as<RawSector>();
  sectors.reserve(raw_data.size());
  for (const auto& raw_sector : raw_data) {
    Sector sector;
    sector.ceiling.height = raw_sector.ceiling_height;
    sector.ceiling.texture = raw_sector.ceiling_texture;
    sector.floor.height = raw_sector.floor_height;
    sector.floor.texture = raw_sector.floor_texture;
    sector.light_level = raw_sector.light_level;
    sectors.emplace_back(sector);
  }
}
void Level::populate_subsectors(const Wad& wad) {
  const Lump& lump = wad.get_lump(name, "SSECTORS");
  std::vector<RawSubsector> raw_data = lump.get_data_as<RawSubsector>();
  subsectors.reserve(raw_data.size());
  for (const auto& raw_subsector : raw_data) {
    Subsector subsector;
    std::size_t offset = static_cast<std::size_t>(raw_subsector.first_seg);
    std::size_t count = static_cast<std::size_t>(raw_subsector.seg_count);

    subsector.segs.reserve(count);
    for (std::size_t i = offset; i < offset + count; ++i)
      subsector.segs.emplace_back(segs.data() + i);

    subsectors.emplace_back(subsector);
  }
}
void Level::populate_segs(const Wad& wad) {
  const Lump& lump = wad.get_lump(name, "SEGS");
  std::vector<RawSeg> raw_data = lump.get_data_as<RawSeg>();
  segs.reserve(raw_data.size());
  for (const auto& raw_seg : raw_data) {
    std::size_t start_index = static_cast<std::size_t>(raw_seg.start_vertex);
    std::size_t end_index = static_cast<std::size_t>(raw_seg.end_vertex);
    std::size_t linedef_index = static_cast<std::size_t>(raw_seg.linedef);

    glm::vec2& start = vertices[start_index];
    glm::vec2& end = vertices[end_index];
    Linedef& linedef = linedefs[linedef_index];
    Sidedef& sidedef = (raw_seg.direction) ? *linedef.front : *linedef.back;
    float angle = doom_angle_to_deg(raw_seg.angle);
    int16_t offset = raw_seg.offset;

    Seg seg{start, end, linedef, sidedef, angle, offset};
    segs.emplace_back(seg);
  }
}
void Level::populate_linedefs(const Wad& wad) {
  const Lump& lump = wad.get_lump(name, "LINEDEFS");
  std::vector<RawLinedef> raw_data = lump.get_data_as<RawLinedef>();
  linedefs.reserve(raw_data.size());
  for (const auto& raw_linedef : raw_data) {
    std::size_t start_index =
        static_cast<std::size_t>(raw_linedef.start_vertex);
    std::size_t end_index = static_cast<std::size_t>(raw_linedef.end_vertex);

    glm::vec2& start = vertices[start_index];
    glm::vec2& end = vertices[end_index];
    Sidedef* front = nullptr;
    Sidedef* back = nullptr;

    // From the doom wiki: " The special value -1 (hexadecimal 0xFFFF) is used
    // to indicate no sidedef, in one-sided lines"
    if (raw_linedef.front_sidedef > 0) {
      std::size_t index = static_cast<std::size_t>(raw_linedef.front_sidedef);
      front = sidedefs.data() + index;
    }
    if (raw_linedef.back_sidedef > 0) {
      std::size_t index = static_cast<std::size_t>(raw_linedef.back_sidedef);
      back = sidedefs.data() + index;
    }

    Linedef linedef{
        start,
        end,
        front,
        back,
    };
    linedefs.emplace_back(linedef);
  }
}
void Level::populate_sidedefs(const Wad& wad) {
  const Lump& lump = wad.get_lump(name, "SIDEDEFS");
  std::vector<RawSidedef> raw_data = lump.get_data_as<RawSidedef>();
  sidedefs.reserve(raw_data.size());
  for (const auto& raw_sidedef : raw_data) {
    // FIXME: Texture names need to have trailing null chars trimmed
    std::string upper_name = raw_sidedef.upper_name;
    std::string lower_name = raw_sidedef.lower_name;
    std::string middle_name = raw_sidedef.middle_name;

    std::size_t sector_index =
        static_cast<std::size_t>(raw_sidedef.sector_facing);
    Sector& sector_facing = sectors[sector_index];
    glm::vec2 offset = {raw_sidedef.x_offset, raw_sidedef.y_offset};

    Sidedef sidedef{
        upper_name, lower_name, middle_name, sector_facing, offset,
    };
    sidedefs.emplace_back(sidedef);
  }
}
void Level::populate_vertices(const Wad& wad) {
  const Lump& lump = wad.get_lump(name, "VERTEXES");
  std::vector<RawVertex> raw_data = lump.get_data_as<RawVertex>();
  vertices.reserve(raw_data.size());
  for (const auto& raw_vertex : raw_data) {
    glm::vec2 vertex{raw_vertex.x_pos, raw_vertex.y_pos};
    vertices.emplace_back(vertex);
  }
}

void Level::populate_nodes(const Wad& wad) {
  const Lump& lump = wad.get_lump(name, "NODES");
  std::vector<RawNode> raw_data = lump.get_data_as<RawNode>();
  nodes.reserve(raw_data.size());

  // Initialize nodes
  for (const auto& raw_node : raw_data) {
    glm::vec2 part_start{
        raw_node.x_part_start,
        raw_node.x_part_start + raw_node.x_part_delta,
    };
    glm::vec2 part_end{
        raw_node.y_part_start,
        raw_node.y_part_start + raw_node.y_part_delta,
    };
    Node node{part_start, part_end};
    nodes.emplace_back(node);
  }
  // Link nodes
  for (std::size_t i = 0; i < nodes.size(); ++i) {
    const RawNode& raw_node = raw_data[i];
    Node& node = nodes[i];
    // Mask out sign bit of both targets
    std::size_t left_target = raw_node.left_child & 0x7fff;
    std::size_t right_target = raw_node.right_child & 0x7fff;
    // If child's sign is negative, it points to a subsector
    // Otherwise, it points to a node
    if (raw_node.left_child < 0)
      node.set_subsector_left(subsectors[left_target]);
    else
      node.set_node_left(nodes[left_target]);
    if (raw_node.right_child < 0)
      node.set_subsector_right(subsectors[right_target]);
    else
      node.set_node_right(nodes[right_target]);
  }
  // "The root node is the highest-numbered entry in the lump"
  // (https://doomwiki.org/wiki/Node)
  bsp_root = &nodes[nodes.size() - 1];
}

void Level::finish_connections() {
  // Connect lines to sectors
  for (auto& line : linedefs) {
    if (line.front) {
      Sector& sector = line.front->sector_facing;
      sector.lines.emplace_back(&line);
    }
    if (line.back) {
      Sector& sector = line.front->sector_facing;
      sector.lines.emplace_back(&line);
    }
  }
}
}  // namespace woop