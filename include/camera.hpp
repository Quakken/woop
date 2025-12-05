/**
 * @file camera.hpp
 * @authors quak
 * @brief Declares the camera class, which is used to transform all world space
 * coordinates to clip and screen space based on a position and rotation.
 */

#pragma once

#include "window.hpp"  /* woop::Window */
#include "glm/glm.hpp" /* glm::ivec2, glm::vec3, glm::vec4, glm::mat4 */

namespace woop {
/**
 * @brief Configuration data for cameras.
 */
struct CameraConfig {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  float rotation = -90.0f;
  float near_plane = 0.1f;
  float far_plane = 10'000.0f;
  float fov = 45.0f;
};

class Camera {
 public:
  Camera(Window& window, const CameraConfig& cfg = CameraConfig{});

  glm::vec3 get_position() const noexcept { return config.position; }
  glm::vec2 get_position_2d() const noexcept {
    return glm::vec2{config.position.x, config.position.z};
  }
  void set_position(const glm::vec3& new_pos) noexcept {
    config.position = new_pos;
  }

  float get_rotation() const noexcept { return config.rotation; }
  void set_rotation(float new_angle) noexcept { config.rotation = new_angle; }

  float get_fov() const noexcept { return config.fov; }
  void set_fov(float new_fov) noexcept { config.fov = new_fov; }
  float get_near_plane() const noexcept { return config.near_plane; }
  void set_near_plane(float new_plane) noexcept {
    config.near_plane = new_plane;
  }
  float get_far_plane() const noexcept { return config.far_plane; }
  void set_far_plane(float new_plane) noexcept { config.far_plane = new_plane; }

  /**
   * @brief Returns a reference to the camera's associated window.
   */
  const Window& get_window() const noexcept { return window; }
  Window& get_window() noexcept { return window; }

 private:
  CameraConfig config;
  Window& window;
};
}  // namespace woop