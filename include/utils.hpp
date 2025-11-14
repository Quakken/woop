#pragma once

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