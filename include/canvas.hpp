#pragma once

#include "window.hpp"  /* woop::Window */
#include "bsp.hpp"     /* woop::Node */
#include "level.hpp"   /* woop::Seg, woop::Subsector */
#include "glad/glad.h" /* OpenGL */
#include "glm/fwd.hpp" /* glm::vec2, glm::vec3 */

namespace woop {
class Canvas;
class Renderer;

/**
 * @brief Describes how an image should be drawn.
 */
enum class DrawMode {
  Solid,
  Wireframe,
  Textured,
};

/**
 * @brief Handles all draw calls for a canvas. When destroyed, presents the
 * output to the screen.
 */
class DrawContext {
 public:
  DrawContext(const DrawContext& other) = delete;
  ~DrawContext();

  DrawContext& operator=(const DrawContext& other) = delete;

  void clear(const glm::vec3& color);
  void draw(DrawMode mode, const Node& node);
  void draw(DrawMode mode, const Subsector& subsector);
  void draw(DrawMode mode, const Seg& seg);

 private:
  friend Canvas;
  DrawContext(Canvas& canvas);

  void draw_column_solid(int top, int bottom);
  void draw_column_wireframe(int top, int bottom);

  glm::vec3* buffer;
};

/**
 * @brief Manage draw operations to write raw pixel data to the screen.
 */
class Canvas {
 public:
  Canvas(Renderer& renderer);
  Canvas(const Canvas& other) = delete;
  ~Canvas();

  Canvas& operator=(const Canvas& other) = delete;

  DrawContext begin_draw();

 private:
  friend DrawContext;

  /*
  // Uncomment this when textures are implemented
  void draw_column_textured(int top, int bottom, const Texture& texture);
  */

  // Raw vertex data used to create a fullscreen quad
  static constexpr glm::vec2 vertex_data[] = {
      {-1.0f, -1.0f}, {0.0f, 0.0f}, /* bottom left */
      {1.0f, -1.0f},  {1.0f, 0.0f}, /* bottom right */
      {1.0f, 1.0f},   {1.0f, 1.0f}, /* top right */
      {-1.0f, 1.0f},  {0.0f, 1.0f}, /* top left */
  };

  Renderer& renderer;
  GLuint pbo;
  GLuint vbo;
  GLuint vao;
  GLuint texture;
};
}  // namespace woop