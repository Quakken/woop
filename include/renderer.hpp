#pragma once

#include "camera.hpp"  /* woop::Camera */
#include "window.hpp"  /* woop::Window */
#include "level.hpp"   /* woop::Level */
#include "canvas.hpp"  /* woop::Canvas */
#include "shader.hpp"  /* woop::Shader */
#include "glad/glad.h" /* OpenGL functions */

namespace woop {
struct RendererConfig {
  Shader shader =
      Shader::from_file("assets/shaders/vert.glsl", "assets/shaders/frag.glsl");
  glm::vec3 clear_color = glm::vec3{0.0f};
  glm::vec3 fill_color = glm::vec3(1.0f);
  unsigned texture_unit = 0;
};

/**
 * @brief Allows levels to be drawn to the screen in a consistent fashion.
 */
class Renderer {
 public:
  Renderer(Window& window,
           Camera& camera,
           const RendererConfig& cfg = RendererConfig{});

  void draw_level(DrawMode mode, const Level& level);

 private:
  friend Canvas;

  RendererConfig config;
  Canvas canvas;
  Window& window;
  Camera& camera;
};
}  // namespace woop