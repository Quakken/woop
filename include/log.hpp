/**
 * @file log.hpp
 * @authors quak
 * @brief Defines a number of functions to log messages at different error
 * levels.
 */

#pragma once
#include <iostream>

namespace woop {
/**
 * @brief Logs a trace message
 */
template <typename... Args>
inline constexpr void log(Args&&... args) {
  // TODO: Colored log messages
  std::cout << "[woop] ";
  ((std::cout << std::forward<Args>(args)), ...);
  std::cout << std::endl;
}

/**
 * @brief Logs a standard message
 */
template <typename... Args>
inline constexpr void log_standard(Args&&... args) {
  std::cout << "[woop] ";
  ((std::cout << std::forward<Args>(args)), ...);
  std::cout << std::endl;
}

/**
 * @brief Logs a warning
 */
template <typename... Args>
inline constexpr void log_warning(Args&&... args) {
  // TODO: Colored log messages
  std::cerr << "[woop:warn] ";
  ((std::cerr << std::forward<Args>(args)), ...);
  std::cerr << std::endl;
}

/**
 * @brief Logs an error
 */
template <typename... Args>
inline constexpr void log_error(Args&&... args) {
  // TODO: Colored log messages
  std::cerr << "[woop:error] ";
  ((std::cerr << std::forward<Args>(args)), ...);
  std::cerr << std::endl;
}

/**
 * @brief Logs a fatal error
 */
template <typename... Args>
inline constexpr void log_fatal(Args&&... args) {
  // TODO: Colored log messages
  std::cerr << "[woop:fatal] ";
  ((std::cerr << std::forward<Args>(args)), ...);
  std::cerr << std::endl;
}
}  // namespace woop