#include "renderer.hpp"
#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include "glm/trigonometric.hpp"
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
  Node::Child nearest_child = node.get_nearest_child(camera.get_position());
  Node::Child farthest_child = !nearest_child;

  // FIXME: Remove this
  std::swap(nearest_child, farthest_child);

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

  // Don't draw "null" segs
  if (invalid || !seg.sidedef)
    return;

  const Sector& sector = seg.sidedef->sector_facing;
  int16_t ceil = sector.ceiling.height;
  int16_t floor = sector.floor.height;

  // Translate world coordinates to be relative to camera
  glm::vec3 cam_pos = camera.get_position();
  glm::vec2 cam_pos_2d = glm::vec2(cam_pos.x, cam_pos.z);
  glm::vec2 start = seg.start - cam_pos_2d;
  glm::vec2 end = seg.end - cam_pos_2d;

  // Get angle to each position
  float start_rad = std::atan2(start.y, start.x);
  float end_rad = std::atan2(end.y, end.x);
  int16_t start_angle = rad_to_doom_angle(start_rad);
  int16_t end_angle = rad_to_doom_angle(end_rad);

  // Transform angle by camera rotation
  start_angle -= deg_to_doom_angle(camera.get_rotation());
  end_angle -= deg_to_doom_angle(camera.get_rotation());

  // Back-face culling
  if (end_angle - start_angle < deg_to_doom_angle(0))
    return;

  // Get scale for output columns
  float start_distance = std::sqrt(start.x * start.x + start.y * start.y);
  float end_distance = std::sqrt(end.x * end.x + end.y * end.y);
  float start_scale = get_column_scale(start_angle, start_distance);
  float end_scale = get_column_scale(end_angle, end_distance);

  int ceil_start =
      static_cast<int>(static_cast<float>(renderer.get_img_size().y) / 2.0f +
                       (ceil - camera.get_position().y) * start_scale);
  int floor_start =
      static_cast<int>(static_cast<float>(renderer.get_img_size().y) / 2.0f +
                       (floor - camera.get_position().y) * start_scale);
  int ceil_end =
      static_cast<int>(static_cast<float>(renderer.get_img_size().y) / 2.0f +
                       (ceil - camera.get_position().y) * end_scale);
  int floor_end =
      static_cast<int>(static_cast<float>(renderer.get_img_size().y) / 2.0f +
                       (floor - camera.get_position().y) * end_scale);

  // Interpolation
  unsigned start_column = angle_to_column(start_angle);
  unsigned end_column = angle_to_column(end_angle);
  // renderer.config.fill_color = Pixel{rand() % 255, 255, rand() % 255, 255};
  for (unsigned col = start_column; col < end_column; ++col) {
    int16_t angle = column_to_angle(col);
    float v = percent_between(angle, start_angle, end_angle);
    int floor_current = interpolate(v, floor_start, floor_end);
    int ceil_current = interpolate(v, ceil_start, ceil_end);
    unsigned floor_row = static_cast<unsigned>(
        std::clamp(floor_current, 0, static_cast<int>(renderer.size.y)));
    unsigned ceil_row = static_cast<unsigned>(
        std::clamp(ceil_current, 0, static_cast<int>(renderer.size.y)));
    for (unsigned row = floor_row; row < ceil_row; ++row) {
      get_buffer_element(col, row) = renderer.config.fill_color;
    }
  }
}

unsigned Frame::angle_to_column(int16_t angle) {
  int16_t fov = deg_to_doom_angle(camera.get_fov());
  angle = std::clamp<int16_t>(angle, -fov / 2, fov / 2);
  return renderer.angle_to_column[angle];
}
int16_t Frame::column_to_angle(unsigned column) {
  return renderer.column_to_angle[column];
}

float Frame::get_column_scale(int16_t angle, float distance) {
  constexpr float min_scale = 0.025f;
  constexpr float max_scale = 64.0f;
  if (angle >= deg_to_doom_angle(90) || angle <= deg_to_doom_angle(-90))
    return 0;
  float screen_plane_distance =
      static_cast<float>(renderer.size.x) / 2.0f /
      static_cast<float>(std::tan(glm::radians(camera.get_fov() / 2.0f)));
  float angle_rad = doom_angle_to_rad(angle);
  float dist_to_screen = screen_plane_distance / cos(angle_rad);
  float scale = std::clamp(dist_to_screen / distance, min_scale, max_scale);
  return scale;
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
  // Assumes that screen width is resolution
  float screen_distance =
      static_cast<float>(size.x) / 2.0f /
      static_cast<float>(std::tan(glm::radians(camera.get_fov() / 2.0f)));
  int16_t fov = deg_to_doom_angle(camera.get_fov());
  float screen_center = static_cast<float>(size.x) / 2.0f;
  // Iterate over every angle within FOV
  for (int16_t angle = -fov / 2; angle <= fov / 2; ++angle) {
    // Calculate the column that the angle intersects with
    float angle_rad = doom_angle_to_rad(angle);
    float col_off = screen_distance * std::tan(angle_rad);
    unsigned col = static_cast<unsigned>(col_off + screen_center);
    angle_to_column.insert({angle, col});
    if (column_to_angle.find(col) == column_to_angle.end())
      column_to_angle.insert({col, angle});
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