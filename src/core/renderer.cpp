#include "renderer.hpp"
#include <algorithm>
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
    : renderer(rndr), display_rect(rndr.display_rect), camera(rndr.camera) {
  map_buffer();
}
Frame::Frame(Frame&& other)
    : renderer(other.renderer),
      display_rect(other.renderer.display_rect),
      camera(other.renderer.camera),
      buffer(other.buffer) {
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
Pixel& Frame::get_buffer_element(int x, int y) {
  if (x < 0 || y < 0 || static_cast<unsigned>(x) > renderer.get_img_size().x ||
      static_cast<unsigned>(y) > renderer.get_img_size().y)
    throw RenderException(RenderException::Type::FrameError,
                          "Can't access pixel at given index (out of bounds)");
  return buffer[y * static_cast<int>(renderer.get_img_size().x) + x];
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
  // Don't draw "null" segs
  if (invalid || !seg.sidedef)
    return;
  const Sector& sector = seg.sidedef->sector_facing;
  int ceil_height = sector.ceiling.height;
  int floor_height = sector.floor.height;

  glm::vec3 w_start_upper = {seg.start.x, ceil_height, seg.start.y};
  glm::vec3 w_start_lower = {seg.start.x, floor_height, seg.start.y};
  glm::vec3 w_end_upper = {seg.end.x, ceil_height, seg.end.y};
  glm::vec3 w_end_lower = {seg.end.x, floor_height, seg.end.y};

  glm::vec4 c_start_upper = camera.world_to_clip(w_start_upper);
  glm::vec4 c_start_lower = camera.world_to_clip(w_start_lower);
  glm::vec4 c_end_upper = camera.world_to_clip(w_end_upper);
  glm::vec4 c_end_lower = camera.world_to_clip(w_end_lower);

  // Entire seg is hidden from view
  if (!camera.is_visible(c_start_upper) && !camera.is_visible(c_start_lower) &&
      !camera.is_visible(c_end_upper) && !camera.is_visible(c_end_lower))
    return;

  std::optional<glm::vec3> n_o_start_upper = camera.clip_to_ndc(c_start_upper);
  std::optional<glm::vec3> n_o_start_lower = camera.clip_to_ndc(c_start_lower);
  std::optional<glm::vec3> n_o_end_upper = camera.clip_to_ndc(c_end_upper);
  std::optional<glm::vec3> n_o_end_lower = camera.clip_to_ndc(c_end_lower);

  // One vertex is invalid
  if (!n_o_start_upper || !n_o_start_lower || !n_o_end_upper ||
      !n_o_end_lower) {
    log_warning("Invalid vertex?");
    return;
  }

  glm::vec3 n_start_upper = n_o_start_upper.value();
  glm::vec3 n_start_lower = n_o_start_lower.value();
  glm::vec3 n_end_upper = n_o_end_upper.value();
  glm::vec3 n_end_lower = n_o_end_lower.value();

  if (n_start_upper.x != n_start_lower.x || n_end_upper.x != n_end_lower.x) {
    // This probably should not happen!
    log_warning("Possible error? X value are not equal.");
  }

  glm::ivec2 s_start_upper = camera.ndc_to_screen(n_start_upper);
  glm::ivec2 s_start_lower = camera.ndc_to_screen(n_start_lower);
  glm::ivec2 s_end_upper = camera.ndc_to_screen(n_end_upper);
  glm::ivec2 s_end_lower = camera.ndc_to_screen(n_end_lower);

  if (s_end_upper.x < s_start_upper.x) {
    std::swap(s_start_upper, s_end_upper);
    std::swap(s_start_lower, s_end_lower);
  }

  renderer.config.fill_color = Pixel(rand() % 255, 255, 255, 255);

  draw_seg_impl(mode, s_start_upper, s_start_lower, s_end_upper, s_end_lower);
}

void Frame::draw_node_child(DrawMode mode,
                            const Node& node,
                            Node::Child child) {
  if (node.is_node(child))
    draw(mode, node.get_node(child));
  else
    draw(mode, node.get_subsector(child));
}
void Frame::draw_seg_impl(DrawMode mode,
                          const glm::ivec2& start_upper,
                          const glm::ivec2& start_lower,
                          const glm::ivec2& end_upper,
                          const glm::ivec2& end_lower) {
  glm::ivec2 diff_upper = end_upper - start_upper;
  glm::ivec2 diff_lower = end_lower - start_lower;
  glm::vec2 f_start_upper = start_upper;
  glm::vec2 f_start_lower = start_lower;
  glm::vec2 f_diff_upper = diff_upper;
  glm::vec2 f_diff_lower = diff_lower;
  if (diff_upper.x != diff_lower.x) {
    // Another possible error? Should not be any angled verticals
    log_warning("Angled vertical!!!");
  }
  for (int x = 0; x < diff_upper.x; ++x) {
    float part_x = static_cast<float>(x) / static_cast<float>(diff_upper.x);
    glm::vec2 f_current_upper = f_start_upper + f_diff_upper * part_x;
    glm::vec2 f_current_lower = f_start_lower + f_diff_lower * part_x;
    glm::ivec2 current_upper = f_current_upper;
    glm::ivec2 current_lower = f_current_lower;
    if (current_upper.x != current_lower.x) {
      log_warning("Angled screen space vertical!!! Before drawing!!!");
    }
    draw_column(mode, current_upper.x, current_upper.y, current_lower.y);
  }
}
void Frame::draw_column(DrawMode mode, int column, int top, int bottom) {
  if (column < 0 || static_cast<unsigned>(column) > renderer.get_img_size().x)
    return;
  switch (mode) {
    case DrawMode::Solid:
      draw_column_solid(column, top, bottom);
      break;
    case DrawMode::Textured:
      log_warning("Textured drawing not implemented.");
      break;
    case DrawMode::Wireframe:
      draw_column_wireframe(column, top, bottom);
      break;
    default:
      log_error("Unknown draw mode given!");
      break;
  }
}
void Frame::draw_column_solid(int column, int top, int bottom) {
  if (column < 0 || static_cast<unsigned>(column) > renderer.get_img_size().x)
    return;
  top = std::clamp(top, 0, static_cast<int>(renderer.get_img_size().y));
  bottom = std::clamp(bottom, 0, static_cast<int>(renderer.get_img_size().y));
  for (int row = bottom; row < top; ++row) {
    Pixel& pixel = get_buffer_element(column, row);
    pixel = renderer.config.fill_color;
  }
}
void Frame::draw_column_wireframe(int column, int top, int bottom) {
  UNUSED_PARAMETER(column);
  UNUSED_PARAMETER(top);
  UNUSED_PARAMETER(bottom);
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