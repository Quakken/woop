#include "display_rect.hpp"
#include "glm/ext/vector_float3.hpp"
#include "log.hpp"
#include "shader.hpp"
#include "renderer.hpp"

namespace woop {
DisplayRect::DisplayRect(const Renderer& rndr, Shader&& shdr)
    : renderer(rndr), shader(std::forward<Shader>(shdr)) {
  texture_unit = renderer.get_texture_unit();
  gen_texture();
  gen_quad();
}
DisplayRect::~DisplayRect() {
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);
  glDeleteTextures(1, &texture);
}

void DisplayRect::draw() const noexcept {
  shader.use();
  shader.set_uniform("u_texture", texture_unit);
  bind_texture();
  bind_vertices();
  glDrawElements(GL_TRIANGLES, sizeof(elements), GL_UNSIGNED_INT, &elements);
}
void DisplayRect::bind_texture() const noexcept {
  glActiveTexture(GL_TEXTURE0 + texture_unit);
  glBindTexture(GL_TEXTURE_2D, texture);
}

void DisplayRect::gen_texture() {
  glGenTextures(1, &texture);
  bind_texture();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderer.get_img_size().x,
               renderer.get_img_size().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               nullptr);
}
void DisplayRect::gen_quad() {
  gen_buffers();
  gen_vertex_array();
}
void DisplayRect::gen_buffers() {
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), &vertex_data,
               GL_STATIC_DRAW);

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), &elements,
               GL_STATIC_DRAW);
}
void DisplayRect::gen_vertex_array() {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                        (GLvoid*)(offsetof(VertexData, pos)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                        (GLvoid*)(offsetof(VertexData, uv)));
}

void DisplayRect::bind_vertices() const noexcept {
  glBindVertexArray(vao);
}
}  // namespace woop