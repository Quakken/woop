/**
 * @file renderer.cpp
 * @authors quak
 * @brief Defines members of the Frame and Renderer classes.
 */

#include "renderer.hpp"          /* woop::Frame, woop::Renderer */
#include "glm/trigonometric.hpp" /* glm::radians, glm::degrees */
#include "utils.hpp"             /* UNUSED_PARAMETER */
#include <algorithm>             /* std::swap, std::clamp */
#include <cstdint>               /* int16_t */
#include <cstdlib>               /* TEMP */
#include <string>                /* std::string */

#include <unordered_map> /* TEMP */

namespace woop {
// TEMP
Pixel get_random_color() {
  return Pixel{rand(), rand(), rand(), 255};
}

Pixel get_texture_color(const std::string& name) {
  static std::unordered_map<std::string, Pixel> colormap;
  if (colormap.find(name) == colormap.end())
    colormap.insert({name, get_random_color()});
  return colormap[name];
}

Shader get_shader_from_cfg(const RendererConfig cfg) {
  if (!cfg.shaders.vert_path.empty() && !cfg.shaders.frag_path.empty())
    return Shader::from_file(cfg.shaders.vert_path, cfg.shaders.frag_path);
  else if (!cfg.shaders.vert_src.empty() && !cfg.shaders.frag_src.empty())
    return Shader{cfg.shaders.vert_src, cfg.shaders.frag_src};
  throw RenderException(RenderException::Type::InvalidConfig,
                        "Unable to create shader (incomplete source/paths)");
}

Frame::Frame(Renderer& rndr)
    : renderer(rndr),
      visible_rows(renderer.get_img_size().x, {0, renderer.get_img_size().y}),
      display_rect(rndr.display_rect),
      camera(rndr.camera),
      invalid(false) {
  map_buffer();
}
Frame::Frame(Frame&& other)
    : renderer(other.renderer),
      visible_rows(renderer.get_img_size().x, {0, renderer.get_img_size().y}),
      display_rect(other.renderer.display_rect),
      camera(other.renderer.camera),
      buffer(other.buffer),
      invalid(false) {
  other.invalid = true;
}
Frame::~Frame() {
  if (invalid)
    return;
  glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
  // Draw frame to the window
  update_display_texture();
  display_rect.draw();
  renderer.window.swap_buffers();
}

bool Frame::is_image_done() const noexcept {
  // All columns have been drawn to:
  // ocluded range = {0, renderer.get_img_size().x};
  if (occluded_cols.size() != 1)
    return false;
  return occluded_cols.front().start == 0 &&
         occluded_cols.front().end == renderer.get_img_size().x;
}
void Frame::insert_occluded_range(unsigned start, unsigned end) noexcept {
  // Invalid range
  if (start > end)
    return;

  // Insert new range while keeping list sorted
  std::list<UnsignedRange>::iterator found = occluded_cols.begin();
  while (found != occluded_cols.end() && found->start < start)
    ++found;
  occluded_cols.emplace(found, UnsignedRange{start, end});

  // Re-sort and merge occlusion list
  std::list<UnsignedRange> new_occluded_cols{occluded_cols.front()};
  for (const auto& range : occluded_cols) {
    UnsignedRange& prev = new_occluded_cols.back();
    if (prev.end >= range.start)
      prev.end = std::max(prev.end, range.end);
    else
      new_occluded_cols.emplace_back(range);
  }
  occluded_cols = new_occluded_cols;
}

std::vector<UnsignedRange> Frame::get_visible_subsegs(unsigned start,
                                                      unsigned end) noexcept {
  std::vector<UnsignedRange> out{UnsignedRange{start, end}};
  for (const auto& range : occluded_cols) {
    UnsignedRange& prev = out.back();
    // Occluded range start might be overlapping with previous range
    if (range.start > prev.start) {
      unsigned prev_end = prev.end;
      // Clip previous range's start if necessary
      prev.end = std::min(prev_end, range.start);
      // Re-insert the rest of the range, if not fully occluded
      if (range.end < end)
        out.emplace_back(UnsignedRange{std::min(prev_end, range.end), end});
    }
    // Occluded range end overlaps with previous range's start
    else if (range.end > prev.start)
      prev.start = range.end;
  }
  return out;
}

void Frame::map_buffer() {
  renderer.bind_pbo_back();
  void* raw_buffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
  buffer = static_cast<Pixel*>(raw_buffer);
}
void Frame::update_display_texture() {
  renderer.swap_pbos();
  renderer.bind_pbo_front();
  display_rect.bind_texture();
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderer.get_img_size().x,
                  renderer.get_img_size().y, GL_RGBA, GL_UNSIGNED_BYTE,
                  nullptr);
}
Pixel& Frame::get_buffer_element(unsigned x, unsigned y) {
  if (x > renderer.get_img_size().x || y > renderer.get_img_size().y)
    throw RenderException(RenderException::Type::FrameError,
                          "Can't access pixel at given index (out of bounds)");
  return buffer[y * renderer.get_img_size().x + x];
}

void Frame::clear(const Pixel& color) {
  if (invalid)
    return;
  std::fill(buffer, buffer + renderer.get_pixel_count(), color);
  occluded_cols.clear();
}

void Frame::draw(DrawMode mode, const Node& node) {
  if (invalid || is_image_done())
    return;
  Node::Child nearest_child = node.get_nearest_child(camera.get_position_2d());
  Node::Child farthest_child = !nearest_child;

  draw_node_child(mode, node, nearest_child);
  draw_node_child(mode, node, farthest_child);
}
void Frame::draw(DrawMode mode, const Subsector& subsector) {
  if (invalid || is_image_done())
    return;
  for (const Seg* seg : subsector.segs)
    draw(mode, *seg);
}
void Frame::draw(DrawMode mode, const Seg& seg) {
  UNUSED_PARAMETER(mode);

  if (invalid || is_image_done() || !seg.sidedef)
    return;

  glm::vec2 start =
      rotate_point(seg.start - camera.get_position_2d(), camera.get_rotation());
  glm::vec2 end =
      rotate_point(seg.end - camera.get_position_2d(), camera.get_rotation());

  if (!is_seg_visible(start, end))
    return;

  float start_screen = get_screen_plane_y(start);
  float end_screen = get_screen_plane_y(end);
  unsigned start_column = get_column(start_screen);
  unsigned end_column = get_column(end_screen);

  // All "subsegments", or visible fragments of the current seg
  std::vector subsegs = get_visible_subsegs(start_column, end_column);
  if (subsegs.size() == 0)
    return;
  draw_subsegs(seg, subsegs, start, end);
  if (is_seg_solid(seg))
    insert_occluded_range(start_column, end_column);
}
void Frame::draw_subsegs(const Seg& seg,
                         const std::vector<UnsignedRange>& subsegs,
                         const glm::vec2& start,
                         const glm::vec2& end) {
  float start_screen = get_screen_plane_y(start);
  float end_screen = get_screen_plane_y(end);
  float start_scale = get_scale(start.x);
  float end_scale = get_scale(end.x);

  Sector& sector = seg.sidedef->sector_facing;
  int16_t floor = sector.floor.height;
  int16_t ceil = sector.ceiling.height;

  for (const auto& subseg : subsegs) {
    for (unsigned col = subseg.start; col < subseg.end; ++col) {
      // Scale interpolation using screen plane coordinates as reference
      float screen = get_screen_plane_y(col);
      float v = static_cast<float>(screen - start_screen) /
                static_cast<float>(end_screen - start_screen);
      float scale = start_scale + v * (end_scale - start_scale);

      // Drawing solid segs
      if (is_seg_solid(seg)) {
        UnsignedRange range = get_row_range(floor, ceil, scale);
        // Clip occluded rows
        range = clip_row_range(col, range);

        // TODO: Remove me!
        renderer.set_fill_color(get_texture_color(seg.sidedef->middle_name));

        draw_column_solid(col, range);
      }
      // Drawing "window" segs
      else {
        Sector& opposite = (seg.sidedef == seg.linedef.front)
                               ? seg.linedef.back->sector_facing
                               : seg.linedef.front->sector_facing;
        int16_t opposite_floor = opposite.floor.height;
        int16_t opposite_ceil = opposite.ceiling.height;
        // We are looking through the "back" of the window (don't draw anything)
        UnsignedRange window_range;
        if (floor > opposite_floor && ceil < opposite_ceil) {
          window_range = get_row_range(floor, ceil, scale);
        }
        // We are looking through the "front" of the window (draw the frame)
        else {
          window_range = get_row_range(opposite_floor, opposite_ceil, scale);
          UnsignedRange bottom_range =
              get_row_range(floor, opposite_floor, scale);
          UnsignedRange top_range = get_row_range(opposite_ceil, ceil, scale);

          bottom_range = clip_row_range(col, bottom_range);
          top_range = clip_row_range(col, top_range);

          // TODO: Remove me!
          renderer.set_fill_color(get_texture_color(seg.sidedef->lower_name));
          draw_column_solid(col, bottom_range);

          // TODO: Remove me!
          renderer.set_fill_color(get_texture_color(seg.sidedef->upper_name));
          draw_column_solid(col, top_range);
        }
        window_range = clip_row_range(col, window_range);
        visible_rows[col].start =
            std::max(window_range.start, visible_rows[col].start);
        visible_rows[col].end =
            std::min(window_range.end, visible_rows[col].end);
      }
    }
  }
}

void Frame::draw_column_solid(unsigned column, const UnsignedRange& range) {
  for (unsigned row = range.start; row < range.end; ++row) {
    get_buffer_element(column, row) = renderer.get_fill_color();
  }
}
bool Frame::is_seg_visible(const glm::vec2& start,
                           const glm::vec2& end) const noexcept {
  // Clip planes
  if (start.x < camera.get_near_plane() && end.x < camera.get_near_plane())
    return false;
  if (start.x > camera.get_far_plane() && end.x > camera.get_far_plane())
    return false;

  // FOV culling
  float fov = camera.get_fov();
  float start_angle = glm::degrees(atan2(start.y, start.x));
  float end_angle = glm::degrees(atan2(end.y, end.x));
  if (start_angle > fov / 2.0f && end_angle > fov / 2.0f)
    return false;
  if (start_angle < -fov / 2.0f && end_angle < -fov / 2.0f)
    return false;

  return true;
}
bool Frame::is_seg_solid(const Seg& seg) const noexcept {
  return !seg.linedef.front ^ !seg.linedef.back;
}
glm::vec2 Frame::rotate_point(const glm::vec2& point,
                              float degrees) const noexcept {
  float rad = glm::radians(degrees);
  return {
      point.x * cos(rad) - point.y * sin(rad),
      point.x * sin(rad) + point.y * cos(rad),
  };
}
float Frame::get_screen_plane_y(const glm::vec2& view) noexcept {
  float slope = view.y / view.x;
  return slope * renderer.get_screen_plane_distance();
}
float Frame::get_screen_plane_y(unsigned column) noexcept {
  const float screen_size = static_cast<float>(renderer.get_img_size().x);
  unsigned reflected = renderer.get_img_size().x - column;
  return static_cast<float>(reflected) - screen_size / 2.0f;
}
unsigned Frame::get_column(float screen_y) {
  const float screen_size = static_cast<float>(renderer.get_img_size().x);
  screen_y = std::clamp(screen_y + screen_size / 2.0f, 0.0f, screen_size);
  // Coordinates need to be reflected (world space increses bottom->top, but
  // columns increase left->right)
  return renderer.get_img_size().x - static_cast<unsigned>(screen_y);
}
float Frame::get_scale(float distance) {
  constexpr float min_scale = 0.0025f;
  constexpr float max_scale = 250'000.0f;
  if (distance <= camera.get_near_plane())
    return max_scale;
  float scale = renderer.get_screen_plane_distance() / distance;
  return std::clamp(scale, min_scale, max_scale);
}
UnsignedRange Frame::get_row_range(int16_t floor, int16_t ceil, float scale) {
  float screen_half = static_cast<float>(renderer.get_img_size().y) / 2.0f;
  float floor_adjusted = (floor - camera.get_position().y) * scale;
  float ceil_adjusted = (ceil - camera.get_position().y) * scale;
  int floor_int = static_cast<int>(screen_half + floor_adjusted);
  int ceil_int = static_cast<int>(screen_half + ceil_adjusted);
  int max_row = static_cast<int>(renderer.get_img_size().y);
  floor_int = std::clamp(floor_int, 0, max_row);
  ceil_int = std::clamp(ceil_int, 0, max_row);
  return {static_cast<unsigned>(floor_int), static_cast<unsigned>(ceil_int)};
}
UnsignedRange Frame::clip_row_range(unsigned column,
                                    const UnsignedRange& range) {
  unsigned start = visible_rows[column].start;
  unsigned end = visible_rows[column].end;
  return {
      std::clamp(range.start, start, end),
      std::clamp(range.end, start, end),
  };
}

void Frame::draw_node_child(DrawMode mode,
                            const Node& node,
                            Node::Child child) {
  if (node.is_node(child))
    draw(mode, node.get_node(child));
  else
    draw(mode, node.get_subsector(child));
}

Renderer::Renderer(Window& wdw, Camera& cam, const RendererConfig& cfg)
    : config(cfg),
      window(wdw),
      camera(cam),
      size(static_cast<unsigned>(window.get_resolution().x),
           static_cast<unsigned>(window.get_resolution().y)),
      display_rect(*this, get_shader_from_cfg(cfg)) {
  // Shaders are only guaranteed to support 16 texture units.
  if (cfg.texture_unit > 16)
    throw RenderException(
        RenderException::Type::InvalidConfig,
        "Attempting to bind renderer to an invalid texture index.");
  screen_plane_distance =
      static_cast<float>(size.x) / 2.0f /
      static_cast<float>(std::tan(glm::radians(camera.get_fov() / 2.0f)));
  gen_pbos();
}

Frame Renderer::begin_frame() {
  Frame frame = Frame(*this);
  frame.clear(config.clear_color);
  return frame;
}
glm::uvec2 Renderer::get_img_size() const noexcept {
  return size;
}
std::size_t Renderer::get_pixel_count() const noexcept {
  return size.x * size.y;
}
unsigned Renderer::get_texture_unit() const noexcept {
  return config.texture_unit;
}

void Renderer::gen_pbos() {
  glGenBuffers(1, &pbo_back);
  bind_pbo_back();
  glBufferData(GL_PIXEL_UNPACK_BUFFER, get_pixel_count() * sizeof(Pixel),
               nullptr, GL_STREAM_DRAW);
  glGenBuffers(1, &pbo_front);
  bind_pbo_front();
  glBufferData(GL_PIXEL_UNPACK_BUFFER, get_pixel_count() * sizeof(Pixel),
               nullptr, GL_STREAM_DRAW);
}
void Renderer::bind_pbo_back() const noexcept {
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_back);
}
void Renderer::bind_pbo_front() const noexcept {
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_front);
}
void Renderer::swap_pbos() noexcept {
  std::swap(pbo_back, pbo_front);
}
}  // namespace woop