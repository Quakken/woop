#version 430 core 
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

out vec2 v_uv;

void main() {
  gl_Position = vec4(a_pos, 1.0f, 1.0f);
  v_uv = a_uv;
}