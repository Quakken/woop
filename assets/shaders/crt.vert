/**
 * @file crt.vert
 * @authors quak
 * @brief Demo of a CRT post-processing effect for woop. Uses blurs, sharpening,
 * and game time to simulate scanlines and color blending.
 */

#version 430 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 uv;

void main() {
  gl_Position = vec4(a_pos, 1.0f, 1.0f);
  uv = a_uv;
}
