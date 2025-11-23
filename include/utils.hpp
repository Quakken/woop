#pragma once

/**
 * @brief Used to suppress compiler errors for unused parameters.
 */
#include <cstdint>
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