#version 430 core
out vec4 f_color;

in vec2 uv;

uniform sampler2D u_texture;

void main() {
  f_color = texture(u_texture, uv);
}