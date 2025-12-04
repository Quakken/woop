/**
 * @file renderer.hpp
 * @authors quak
 * @brief Declares the Renderer and Frame classes, which allow levels to be
 * drawn to the screen based on camera position.
 */

#pragma once

#include "camera.hpp"       /* woop::Camera */
#include "window.hpp"       /* woop::Window */
#include "level.hpp"        /* woop::Level */
#include "display_rect.hpp" /* woop::DisplayRect */
#include "exception.hpp"    /* woop::Exception */
#include "glad/glad.h"      /* OpenGL functions */
#include <optional>         /* std::optional */

namespace woop {
class Renderer;

/**
 * @brief Pixel data in woop (4 bytes, 0->255)
 */
using Pixel = glm::vec<4, std::byte>;

/**
 * @brief Describes how an image should be drawn.
 */
enum class DrawMode {
  Solid,
  Wireframe,
  Textured,
};

struct UnsignedRange {
  unsigned start;
  unsigned end;
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

  /**
   * @brief Clears the frame, setting all pixels to the same color.
   */
  void clear(const Pixel& color);
  /**
   * @brief Draws a node to the frame.
   */
  void draw(DrawMode mode, const Node& node);
  /**
   * @brief Draws a subsector to the frame.
   */
  void draw(DrawMode mode, const Subsector& subsector);
  /**
   * @brief Draws a segg to the frame
   */
  void draw(DrawMode mode, const Seg& seg);

 private:
  friend Renderer;
  Frame(Renderer& renderer);

  /**
   * @brief Returns true if all columns of the output image have been drawn to.
   */
  bool is_image_done() const noexcept;

  void insert_occluded_range(unsigned start, unsigned end) noexcept;
  std::vector<UnsignedRange> get_visible_subsegs(unsigned start,
                                                 unsigned end) noexcept;
  bool clip_seg(glm::vec2& start, glm::vec2& end) noexcept;
  static std::optional<glm::vec2> get_segment_intersection(
      const glm::vec2& start_1,
      const glm::vec2& end_1,
      const glm::vec2& start_2,
      const glm::vec2& end_2) noexcept;

  /**
   * @brief Draws the child of a node.
   */
  void draw_node_child(DrawMode mode, const Node& node, Node::Child child);
  /**
   * @brief Draws all visible fragments of a (potentially) partially occluded
   * seg.
   */
  void draw_subsegs(const Seg& seg,
                    const std::vector<UnsignedRange>& subsegs,
                    const glm::vec2& start,
                    const glm::vec2& end);

  /**
   * @brief Maps the pixel buffer in OpenGL, allowing data to be written.
   */
  void map_buffer();
  /**
   * @brief Updates the texture that the output image is bound to.
   */
  void update_display_texture();
  /**
   * @brief Returns the pixel at the given coordinates.
   */
  Pixel& get_buffer_element(unsigned x, unsigned y);

  /**
   * @brief Draws a solid column to the screen.
   */
  void draw_column_solid(unsigned column, const UnsignedRange& rows);
  /**
   * @brief Draws a wireframe column to the screen.
   */
  void draw_column_wireframe(unsigned column,
                             const UnsignedRange& rows) = delete;

  /**
   * @brief Returns true if a seg can be seen from the player's current
   * position.
   */
  bool is_seg_visible(const glm::vec2& start,
                      const glm::vec2& end) const noexcept;
  /**
   * @brief Returns true if a seg is opaque.
   */
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
   * distance from the camera.
   */
  float get_scale(float distance);
  /**
   * @brief Returns the first (x) and last (y) rows of a column that should be
   * drawn, given a floor, ceiling, and scale factor.
   */
  UnsignedRange get_row_range(int16_t floor, int16_t ceil, float scale);
  /**
   * @brief Clips the given column range based on which rows are currently
   * visible.
   */
  UnsignedRange clip_row_range(unsigned column, const UnsignedRange& range);

  std::list<UnsignedRange> occluded_cols;
  Renderer& renderer;
  std::vector<UnsignedRange> visible_rows;
  DisplayRect& display_rect;
  Camera& camera;
  Pixel* buffer;
  bool invalid;
};

/**
 * @brief Exception thrown when a Renderer or Frame encounters an error.
 */
class RenderException : public Exception {
 public:
  /**
   * @brief Type of exception encountered.
   */
  enum class Type {
    InvalidConfig,
    FrameError,
  };
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

  /**
   * @brief Creates a new frame to be drawn to the screen.
   */
  Frame begin_frame();

  /**
   * @brief Returns the size of the output image.
   */
  glm::uvec2 get_img_size() const noexcept;
  /**
   * @brief Returns the number of pixels in the output image.
   */
  std::size_t get_pixel_count() const noexcept;
  /**
   * @brief Returns the OpenGL texture unit that the output image is bound to.
   */
  unsigned get_texture_unit() const noexcept;
  /**
   * @brief Returns the distance from the screen plane to the camera.
   * The screen plane is the view-space plane where 1 unit = 1 screen column.
   */
  float get_screen_plane_distance() const noexcept {
    return screen_plane_distance;
  }
  /**
   * @brief Sets the renderer's fill color.
   */
  void set_fill_color(const Pixel& color) noexcept {
    config.fill_color = color;
  }
  /**
   * @brief Returns the renderer's fill color.
   */
  Pixel get_fill_color() const noexcept { return config.fill_color; }
  /**
   * @brief Sets the renderer's clear color.
   */
  void set_clear_color(const Pixel& color) noexcept {
    config.clear_color = color;
  }
  /**
   * @brief Returns the renderer's clear color.
   */
  Pixel get_clear_color() const noexcept { return config.clear_color; }

 private:
  friend Frame;

  /**
   * @brief Generates the front and back PBOs with OpenGL.
   */
  void gen_pbos();
  /**
   * @brief Binds the back (hidden) PBO in OpenGL.
   */
  void bind_pbo_back() const noexcept;
  /**
   * @brief Binds the front (shown) PBO in OpenGL.
   */
  void bind_pbo_front() const noexcept;
  /**
   * @brief Swaps the front and back PBOs.
   */
  void swap_pbos() noexcept;

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