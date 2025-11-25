#pragma once

#include <cstdint>                      /* int16_t */
#include "glm/ext/scalar_constants.hpp" /* glm::pi */

/**
 * @brief Used to suppress compiler errors for unused parameters.
 */
#define UNUSED_PARAMETER(param) static_cast<void>(param)

/**
 * @brief Returns true if a value is between a given lower and upper bounds.
 */
template <typename T>
constexpr bool is_between(const T& val, const T& lower, const T& upper) {
  return (val > lower) && (val < upper);
}
/**
 * @brief Returns true if a value is between a given lower and upper bounds
 * (including endpoints).
 */
template <typename T>
constexpr bool is_between_inclusive(const T& val,
                                    const T& lower,
                                    const T& upper) {
  return (val >= lower) && (val <= upper);
}

constexpr float doom_angle_to_deg(int16_t angle) {
  return static_cast<float>(angle) * 180.0f / 32767.0f;
}

constexpr float doom_angle_to_rad(int16_t angle) {
  return static_cast<float>(angle) * glm::pi<float>() / 32767.0f;
}

constexpr int16_t deg_to_doom_angle(float angle) {
  return static_cast<int16_t>(angle * 32767.0f / 180.0f);
}

constexpr int16_t rad_to_doom_angle(float angle) {
  return static_cast<int16_t>(angle * 32767.0f / glm::pi<float>());
}