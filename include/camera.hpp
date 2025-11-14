#pragma once

#include "glm/glm.hpp" /* glm::vec3, glm::mat4 */
#include "window.hpp"  /* glm::Window */
#include <optional>    /* std::optional */

namespace woop {
/**
 * @brief Configuration data for cameras.
 */
struct CameraConfig {
  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  float rotation = -90.0f;
  float near_plane = 0.1f;
  float far_plane = 1'000.0f;
  float fov = 45.0f;
};

class Camera {
 public:
  Camera(const Window& window, const CameraConfig& cfg = CameraConfig{});

  glm::vec3 get_position() const noexcept { return config.position; }
  void set_position(const glm::vec3& new_pos) noexcept;

  float get_rotation() const noexcept { return config.rotation; }
  void set_rotation(float new_angle) noexcept;

  /**
   * @brief Returns the camera's world->clip space transformation matrix.
   */
  const glm::mat4& get_transform() noexcept;

  /**
   * @brief Transforms a point from world space to clip space.
   * @note This operation does not perform perspective division.
   */
  glm::vec4 world_to_clip(const glm::vec3& world) noexcept;

  /**
   * @brief Returns true if a point in clip space is visible.
   */
  bool is_visible(const glm::vec4& clip) const noexcept;

 private:
  /**
   * @brief Updates the camera's transformation matrix.
   */
  void update_transform() noexcept;

  glm::mat4 transform;
  CameraConfig config;
  bool is_dirty;

  const Window& window;
};
}  // namespace woop