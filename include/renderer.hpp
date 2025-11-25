#pragma once

#include "camera.hpp"       /* woop::Camera */
#include "window.hpp"       /* woop::Window */
#include "level.hpp"        /* woop::Level */
#include "display_rect.hpp" /* woop::DisplayRect */
#include "exception.hpp"    /* woop::Exception */
#include "glad/glad.h"      /* OpenGL functions */

#include "unordered_map"

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
  Pixel& get_buffer_element(unsigned x, unsigned y);

  unsigned angle_to_column(int16_t angle);
  int16_t column_to_angle(unsigned column);
  static constexpr float percent_between(float value, float start, float end) {
    return std::clamp((value - start) / (end - start), 0.0f, 1.0f);
  }
  template <typename T>
  static constexpr T interpolate(float v, T start, T end) {
    return +static_cast<T>(static_cast<float>(start) +
                           v * static_cast<float>(end - start));
  }

  float get_column_scale(int16_t angle, float distance);
  float get_point_distance(const glm::vec2& point);

  void draw_node_child(DrawMode mode, const Node& node, Node::Child child);

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
  Pixel clear_color = Pixel{0, 0, 0, 255};
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
  void gen_lookup_tables();
  void bind_pbo_back() const noexcept;
  void bind_pbo_front() const noexcept;
  void swap_pbos() noexcept;

  std::unordered_map<int16_t, unsigned> angle_to_column;
  std::unordered_map<unsigned, int16_t> column_to_angle;
  RendererConfig config;
  Window& window;
  Camera& camera;
  glm::uvec2 size;
  DisplayRect display_rect;
  unsigned pbo_back;
  unsigned pbo_front;
};
}  // namespace woop