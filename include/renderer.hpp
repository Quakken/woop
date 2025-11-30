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

  void draw_node_child(DrawMode mode, const Node& node, Node::Child child);

  void map_buffer();
  void update_display_texture();
  Pixel& get_buffer_element(unsigned x, unsigned y);

  void draw_column_solid(unsigned column, unsigned bottom, unsigned top);
  void draw_column_wireframe(unsigned column,
                             unsigned bottom,
                             unsigned top) = delete;

  bool is_seg_visible(const glm::vec2& start,
                      const glm::vec2& end) const noexcept;
  bool is_seg_solid(const Seg& seg) const noexcept;
  /**
   * @brief Rotates a point around the origin.
   */
  glm::vec2 rotate_point(const glm::vec2& point, float degrees) const noexcept;
  /**
   * @brief Returns the y coordinate of a point when projected to screen plane
   * coordinates.
   */
  float get_screen_plane_y(const glm::vec2& view) noexcept;
  /**
   * @brief Converts screen column to world space at the screen plane.
   */
  float get_screen_plane_y(unsigned column) noexcept;
  /**
   * @brief Returns the column that a point (in screen plane coordinates) should
   * be drawn at. The given value is clamped to a visible range.
   */
  unsigned get_column(float screen_y);
  /**
   * @brief Returns the column that a point should be drawn at. The given value
   * is clamped to a visible range.
   */
  unsigned get_column(const glm::vec2& view);
  /**
   * @brief Returns the scale that a point should be drawn at, given its
   * horizontal distance from the camera.
   */
  float get_scale(float distance);
  /**
   * @brief Returns the first (x) and last (y) rows of a column that should be
   * drawn, given a floor, ceiling, and scale factor.
   */
  glm::uvec2 get_column_range(int16_t floor, int16_t ceil, float scale);

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
  float get_screen_plane_distance() const noexcept {
    return screen_plane_distance;
  }
  void set_fill_color(const Pixel& color) noexcept {
    config.fill_color = color;
  }
  Pixel get_fill_color() const noexcept { return config.fill_color; }
  void set_clear_color(const Pixel& color) noexcept {
    config.clear_color = color;
  }
  Pixel get_clear_color() const noexcept { return config.clear_color; }

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
  float screen_plane_distance;
};
}  // namespace woop