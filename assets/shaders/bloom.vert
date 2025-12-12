/**
 * @file bloom.frag
 * @authors quak
 * @brief Demo showcasing a simple bloom effect for woop. Colors that pass a
 * brightness threshold appear to glow, while other colors are darkened.
 */

#version 430 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 uv;

void main() {
  gl_Position = vec4(a_pos, 1.0f, 1.0f);
  uv = a_uv;
}
