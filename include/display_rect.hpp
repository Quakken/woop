#pragma once

#include "shader.hpp"   /* woop::Shader */
#include "glad/glad.h"  /* OpenGL */
#include "glm/vec2.hpp" /* glm::vec2 */

namespace woop {
class Renderer;
/**
 * @brief Manage draw operations to write raw pixel data to the screen.
 */
class DisplayRect {
 public:
  DisplayRect(const Renderer& renderer, Shader&& shader);
  DisplayRect(const DisplayRect& other) = delete;
  ~DisplayRect();

  DisplayRect& operator=(const DisplayRect& other) = delete;

  /**
   * @brief Returns a Frame, which can be used to write to the output
   * frame.
   */
  void draw() const noexcept;

  void bind_texture() const noexcept;

 private:
  void gen_texture();
  void gen_quad();
  void gen_buffers();
  void gen_vertex_array();

  void bind_vertices() const noexcept;

  struct VertexData {
    glm::vec2 pos;
    glm::vec2 uv;
  };
  // Raw vertex data used to create a fullscreen quad
  inline const static VertexData vertex_data[] = {
      {{-1.0f, -1.0f}, {0.0f, 0.0f}}, /* bottom left */
      {{1.0f, -1.0f}, {1.0f, 0.0f}},  /* bottom right */
      {{1.0f, 1.0f}, {1.0f, 1.0f}},   /* top right */
      {{-1.0f, 1.0f}, {0.0f, 1.0f}},  /* top left */
  };

  static constexpr unsigned elements[] = {0, 1, 2, 0, 2, 3};

  const Renderer& renderer;
  Shader shader;
  GLuint vbo;
  GLuint ebo;
  GLuint vao;
  GLuint texture;
  unsigned texture_unit;
};
}  // namespace woop