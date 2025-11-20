#pragma once

#include "camera.hpp"       /* woop::Camera */
#include "window.hpp"       /* woop::Window */
#include "level.hpp"        /* woop::Level */
#include "display_rect.hpp" /* woop::DisplayRect */
#include "exception.hpp"    /* woop::Exception */
#include "glad/glad.h"      /* OpenGL functions */

namespace woop {
class Renderer;

using Pixel = glm::vec<4, std::byte>;

/**
 * @brief Describes how an image should be drawn.
 */
enum class DrawMode {
  Solid,
  Wireframe,
  Textured,
};

/**
 * @brief Manages all draw calls for a single frame. When destroyed, draws the
 * frame to the screen.
 */
class Frame {
 public:
  Frame(const Frame& other) = delete;
  Frame(Frame&& other);
  ~Frame();

  Frame& operator=(const Frame& other) = delete;

  void clear(const Pixel& color);
  void draw(DrawMode mode, const Node& node);
  void draw(DrawMode mode, const Subsector& subsector);
  void draw(DrawMode mode, const Seg& seg);

 private:
  friend Renderer;
  Frame(Renderer& renderer);

  void map_buffer();
  void update_display_texture();

  void draw_node_child(DrawMode mode, const Node& node, Node::Child child);
  void draw_column_solid(std::size_t column,
                         std::size_t top,
                         std::size_t bottom);
  void draw_column_wireframe(std::size_t column,
                             std::size_t top,
                             std::size_t bottom);

  Renderer& renderer;
  DisplayRect& display_rect;
  Camera& camera;
  Pixel* buffer;
  bool invalid;
};

class RenderException : public Exception {
 public:
  enum class Type { InvalidConfig, FrameError };
  RenderException(Type type, const std::string_view& what)
      : Exception(what), t(type) {}
  Type type() const noexcept { return t; }

 private:
  Type t;
};

struct RendererConfig {
  struct {
    std::filesystem::path vert_path = "assets/shaders/vert.glsl";
    std::filesystem::path frag_path = "assets/shaders/frag.glsl";
    std::string vert_src = "";
    std::string frag_src = "";
  } shaders;
  Pixel clear_color = Pixel{255, 255, 255, 255};
  Pixel fill_color = Pixel{255, 255, 255, 255};
  unsigned texture_unit = 0;
};

/**
 * @brief Allows objects to be drawn to the screen. Coordinates underlying
 * OpenGL draw calls.
 */
class Renderer {
 public:
  Renderer(Window& window,
           Camera& camera,
           const RendererConfig& cfg = RendererConfig{});

  Frame begin_frame();

  glm::uvec2 get_img_size() const noexcept;
  std::size_t get_pixel_count() const noexcept;
  unsigned get_texture_unit() const noexcept;

 private:
  friend Frame;

  void gen_pbos();
  void bind_pbo_back() const noexcept;
  void bind_pbo_front() const noexcept;
  void swap_pbos() noexcept;

  RendererConfig config;
  Window& window;
  Camera& camera;
  glm::uvec2 size;
  DisplayRect display_rect;
  unsigned pbo_back;
  unsigned pbo_front;
};
}  // namespace woop