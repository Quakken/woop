#include "renderer.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include "glm/trigonometric.hpp"
#include "glm/geometric.hpp"
#include "log.hpp"
#include "shader.hpp"
#include "utils.hpp"
#include "window.hpp"

namespace woop {
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
      display_rect(rndr.display_rect),
      camera(rndr.camera),
      invalid(false) {
  map_buffer();
}
Frame::Frame(Frame&& other)
    : renderer(other.renderer),
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
  for (std::size_t i = 0; i < renderer.get_pixel_count(); ++i)
    buffer[i] = color;
}

void Frame::draw(DrawMode mode, const Node& node) {
  if (invalid)
    return;
  Node::Child nearest_child = node.get_nearest_child(camera.get_position_2d());
  Node::Child farthest_child = !nearest_child;

  // FIXME: Remove this
  // std::swap(nearest_child, farthest_child);

  draw_node_child(mode, node, nearest_child);
  draw_node_child(mode, node, farthest_child);
}
void Frame::draw(DrawMode mode, const Subsector& subsector) {
  if (invalid)
    return;
  for (const Seg* seg : subsector.segs)
    draw(mode, *seg);
}
void Frame::draw(DrawMode mode, const Seg& seg) {
  UNUSED_PARAMETER(mode);

  if (invalid || !seg.sidedef)
    return;
  // Only draw solid segs for now
  if (!is_seg_solid(seg))
    return;

  glm::vec2 start =
      rotate_point(seg.start - camera.get_position_2d(), camera.get_rotation());
  glm::vec2 end =
      rotate_point(seg.end - camera.get_position_2d(), camera.get_rotation());

  // TODO: Remove me!
  static std::unordered_map<std::string, Pixel> colors;
  if (colors.find(seg.sidedef->middle_name) == colors.end()) {
    colors.insert({seg.sidedef->middle_name,
                   Pixel{rand() % 255, rand() % 255, 255, 255}});
  }
  renderer.set_fill_color(colors[seg.sidedef->middle_name]);

  if (!is_seg_visible(start, end))
    return;

  const Sector& sector = seg.sidedef->sector_facing;
  const int16_t ceil = sector.ceiling.height;
  const int16_t floor = sector.floor.height;

  float start_screen = get_screen_plane_y(start);
  float end_screen = get_screen_plane_y(end);
  unsigned start_column = get_column(start_screen);
  unsigned end_column = get_column(end_screen);

  float start_scale = get_scale(start.x);
  float end_scale = get_scale(end.x);

  for (unsigned col = start_column; col < end_column; ++col) {
    // Scale interpolation using screen plane coordinates as reference
    float screen = get_screen_plane_y(col);
    float v = static_cast<float>(screen - start_screen) /
              static_cast<float>(end_screen - start_screen);
    float scale = start_scale + v * (end_scale - start_scale);

    glm::uvec2 range = get_column_range(floor, ceil, scale);
    draw_column_solid(col, range.x, range.y);
  }
}

void Frame::draw_column_solid(unsigned column, unsigned bottom, unsigned top) {
  for (unsigned row = bottom; row < top; ++row) {
    get_buffer_element(column, row) = renderer.get_fill_color();
  }
}
bool Frame::is_seg_visible(const glm::vec2& start,
                           const glm::vec2& end) const noexcept {
  if (start.x < 0 && end.x < 0)
    return false;

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
  if (distance == 0)
    return max_scale;
  float scale = renderer.get_screen_plane_distance() / distance;
  return std::clamp(scale, min_scale, max_scale);
}
glm::uvec2 Frame::get_column_range(int16_t floor, int16_t ceil, float scale) {
  float screen_half = static_cast<float>(renderer.get_img_size().y) / 2.0f;
  float floor_adjusted = (floor - camera.get_position().y) * scale;
  float ceil_adjusted = (ceil - camera.get_position().y) * scale;
  int floor_int = static_cast<int>(screen_half + floor_adjusted);
  int ceil_int = static_cast<int>(screen_half + ceil_adjusted);
  int max_row = static_cast<int>(renderer.get_img_size().y);
  floor_int = std::clamp(floor_int, 0, max_row);
  ceil_int = std::clamp(ceil_int, 0, max_row);
  return {floor_int, ceil_int};
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
  gen_lookup_tables();
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
void Renderer::gen_lookup_tables() {
  // Populate lookup table for angle->column based on resolution and FOV
  int16_t fov = deg_to_doom_angle(camera.get_fov());
  float screen_center = static_cast<float>(size.x) / 2.0f;
  // Iterate over every angle within FOV
  for (int16_t angle = -fov / 2; angle <= fov / 2; ++angle) {
    // Calculate the column that the angle intersects with
    float angle_rad = doom_angle_to_rad(angle);
    float col_off = screen_plane_distance * std::tan(angle_rad);
    unsigned col = static_cast<unsigned>(col_off + screen_center);
    angle_to_column.insert({angle, col});
    column_to_angle[col] = angle;
  }
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