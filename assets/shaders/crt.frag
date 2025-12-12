/**
 * @file crt.frag
 * @authors quak
 * @brief Demo of a CRT post-processing effect for woop. Uses blurs, sharpening,
 * and game time to simulate scanlines and color blending.
 */

#version 430 core
out vec4 f_color;

in vec2 uv;

uniform sampler2D u_texture;
uniform float u_time;

const float kernel_step = 1.0f / 320.0f;
const vec2 kernel_offsets[9] = {
    vec2(-kernel_step, kernel_step),   // top left
    vec2(0.0f, kernel_step),           // top center
    vec2(kernel_step, kernel_step),    // top right
    vec2(-kernel_step, 0.0f),          // center left
    vec2(0.0f, 0.0f),                  // center center
    vec2(kernel_step, 0.0f),           // center right
    vec2(-kernel_step, -kernel_step),  // bottom left
    vec2(0.0f, -kernel_step),          // bottom center
    vec2(kernel_step, -kernel_step),   // bottom right
};

const float scan_scroll_rate = 2.5f;
const float scan_size = 1.15f;
const float scan_intensity = 1.15f;

void kernels() {
  // Sample adjacent colors
  vec3 adjacent[9];
  for (int i = 0; i < 9; ++i) {
    vec4 color = texture(u_texture, uv + kernel_offsets[i]);
    adjacent[i] = vec3(color);
  }

  // Gaussian blur kernel
  float gaussian[9] = {
      1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,  //
      2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,  //
      1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,  //
  };
  // Sharpen kernel
  float sharpen[9] = {
      0.0f,  -1.0f, 0.0f,   //
      -1.0f, 5.0f,  -1.0f,  //
      0.0f,  -1.0f, 0.0f,   //
  };

  // Calculate result of gaussian and sharpen
  vec3 gaussian_result;
  vec3 sharpen_result;
  for (int i = 0; i < 9; ++i) {
    gaussian_result += adjacent[i] * gaussian[i];
    sharpen_result += adjacent[i] * sharpen[i];
  }

  // Combine result of both kernels
  vec3 sharpened_blur = mix(gaussian_result, sharpen_result, 0.5f);
  f_color = vec4(sharpened_blur, 1.0f);
}

void rows() {
  float y_pos = gl_FragCoord.y;
  f_color *=
      scan_intensity * sin(scan_size * (y_pos - u_time * scan_scroll_rate));
}

void main() {
  kernels();
  rows();
}