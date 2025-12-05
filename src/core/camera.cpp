/**
 * @file camera.cpp
 * @authors quak
 * @brief Defines members of the camera class.
 */

#include "camera.hpp"
#include "utils.hpp"                    /* is_between_inclusive */
#include "glm/gtc/matrix_transform.hpp" /* glm::lookAt, glm::perspective */

namespace woop {
Camera::Camera(Window& wdw, const CameraConfig& cfg)
    : config(cfg), window(wdw) {}
}  // namespace woop