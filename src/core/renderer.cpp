#include "renderer.hpp"
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

void Frame::clear(const Pixel& color) {
  if (invalid)
    return;
  // TODO: Uncomment me!
  // for (std::size_t i = 0; i < renderer.get_pixel_count(); ++i)
  //   buffer[i] = color;

  // Testing color output.
  UNUSED_PARAMETER(color);
  for (std::size_t x = 0; x < renderer.get_img_size().x; ++x) {
    for (std::size_t y = 0; y < renderer.get_img_size().y; ++y) {
      buffer[renderer.get_img_size().x * y + x] =
          Pixel{x % 255, y % 255, 0, 255};
    }
  }
}

void Frame::draw(DrawMode mode, const Node& node) {
  if (invalid)
    return;
  Node::Child nearest_child = node.get_nearest_child(camera.get_position());
  Node::Child farthest_child = !nearest_child;
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
  if (invalid)
    return;
  UNUSED_PARAMETER(mode);
  UNUSED_PARAMETER(seg);
  // TODO: Draw segs
}

void Frame::draw_node_child(DrawMode mode,
                            const Node& node,
                            Node::Child child) {
  if (node.is_node(child))
    draw(mode, node.get_node(child));
  else
    draw(mode, node.get_subsector(child));
}
void Frame::draw_column_solid(std::size_t column,
                              std::size_t top,
                              std::size_t bottom) {
  UNUSED_PARAMETER(column);
  UNUSED_PARAMETER(top);
  UNUSED_PARAMETER(bottom);
}
void Frame::draw_column_wireframe(std::size_t column,
                                  std::size_t top,
                                  std::size_t bottom) {
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