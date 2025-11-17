#include "camera.hpp"
#include "utils.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/trigonometric.hpp"

namespace woop {
Camera::Camera(const Window& wdw, const CameraConfig& cfg)
    : config(cfg), is_dirty(true), window(wdw) {}

void Camera::set_position(const glm::vec3& new_pos) noexcept {
  config.position = new_pos;
  is_dirty = true;
}

void Camera::set_rotation(float new_angle) noexcept {
  config.rotation = new_angle;
  is_dirty = true;
}

const glm::mat4& Camera::get_transform() noexcept {
  if (is_dirty)
    update_transform();
  return transform;
}

glm::vec4 Camera::world_to_clip(const glm::vec3& world) noexcept {
  glm::vec4 homogenous = {world, 1.0f};
  homogenous = get_transform() * homogenous;
  return homogenous;
}
glm::vec3 Camera::clip_to_ndc(const glm::vec4& clip) noexcept {
  glm::vec3 out = clip;
  return out / clip.w;
}

void Camera::update_transform() noexcept {
  constexpr glm::vec3 world_up{0.0f, 1.0f, 0.0f};
  const glm::vec3& cam_pos = config.position;

  // Rotation around the X axis
  constexpr float pitch = glm::radians(0.0f);
  // Rotation around the Y axis
  float yaw = glm::radians(config.rotation);
  glm::vec3 cam_fwd = {
      cos(pitch) * cos(yaw),
      sin(pitch),
      cos(pitch) * sin(yaw),
  };

  glm::mat4 view = glm::lookAt(cam_pos, cam_pos - cam_fwd, world_up);
  glm::mat4 projection =
      glm::perspective(glm::radians(config.fov), window.get_aspect_ratio(),
                       config.near_plane, config.far_plane);

  transform = projection * view;
  is_dirty = false;
}

bool Camera::is_visible(const glm::vec4& clip) const noexcept {
  return is_between_inclusive(clip.x, -clip.w, clip.w) &&
         is_between_inclusive(clip.y, -clip.w, clip.w) &&
         is_between_inclusive(clip.z, -clip.w, clip.w);
}
}  // namespace woop