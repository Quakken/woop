/**
 * @file bloom.vert
 * @authors quak
 * @brief Demo showcasing a simple bloom effect for woop. Colors that pass a
 * brightness threshold appear to glow, while other colors are darkened.
 * @note While using this shader, you might want to disable fog. It can cause
 * some strange artifacts when the fog causes a color to pass below the bloom
 * threshold.
 */

#version 430 core
out vec4 f_color;

in vec2 uv;

uniform sampler2D u_texture;

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

// Strength of the bloom effect
const float bloom_strength = 0.85f;
// How bright a pixel needs to be to glow
const float threshold = 0.65f;
// How much to "dull" colors that don't cross the bloom threshold
const float dulling = 0.5f;

/**
 * @brief Returns the given color if it is above the brightness threshold,
 * otherwise returns black.
 */
vec3 clamp_brightness(vec3 color) {
  float brightness = (color.r + color.g + color.b) / 3.0f;
  return step(threshold, brightness) * color;
}

void main() {
  vec4 tex_color = texture(u_texture, uv);

  // Sample adjacent colors, matching their brightness against the bloom
  // threshold
  vec3 thresh_adjacent[9];
  for (int i = 0; i < 9; ++i) {
    vec4 color = texture(u_texture, uv + kernel_offsets[i]);
    thresh_adjacent[i] = clamp_brightness(vec3(color));
  }
  // Gaussian blur kernel
  float gaussian[9] = {
      1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,  //
      2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,  //
      1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,  //
  };
  vec3 gaussian_result;
  for (int i = 0; i < 9; ++i) {
    gaussian_result += thresh_adjacent[i] * gaussian[i];
  }

  vec4 bloom_result = bloom_strength * vec4(gaussian_result, 0.0f);
  f_color = dulling * tex_color + bloom_result;
}