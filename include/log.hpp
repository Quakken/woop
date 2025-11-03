/**
 * @file log.hpp
 * @authors quak
 * @brief Defines a number of functions to log messages at different error
 * levels.
 */

#pragma once
#include "utils.hpp"
#include <iostream>

namespace woop {
/**
 * @brief Logs a trace message
 */
template <typename... Args>
inline constexpr void log(Args&&... args) {
#ifdef WOOP_ENABLE_LOGGING
#ifdef WOOP_COLOR_LOGGING
  std::cout << "\x1b[2m";  // Faint text
#endif
  std::cout << "[woop] ";
  ((std::cout << std::forward<Args>(args)), ...);
  std::cout << std::endl;
#ifdef WOOP_COLOR_LOGGING
  std::cout << "\x1b[0m";
#endif
#else
  (UNUSED_PARAMETER(args), ...);
#endif
}

/**
 * @brief Logs a standard message
 */
template <typename... Args>
inline constexpr void log_standard(Args&&... args) {
#ifdef WOOP_ENABLE_LOGGING
  std::cout << "[woop] ";
  ((std::cout << std::forward<Args>(args)), ...);
  std::cout << std::endl;
#else
  (UNUSED_PARAMETER(args), ...);
#endif
}

/**
 * @brief Logs a warning
 */
template <typename... Args>
inline constexpr void log_warning(Args&&... args) {
#ifdef WOOP_ENABLE_LOGGING
#ifdef WOOP_COLOR_LOGGING
  std::cout << "\x1b[33m";  // Yellow text
#endif
  std::cerr << "[woop:warn] ";
  ((std::cerr << std::forward<Args>(args)), ...);
  std::cerr << std::endl;
#ifdef WOOP_COLOR_LOGGING
  std::cout << "\x1b[0m";
#endif
#else
  (UNUSED_PARAMETER(args), ...);
#endif
}

/**
 * @brief Logs an error
 */
template <typename... Args>
inline constexpr void log_error(Args&&... args) {
#ifdef WOOP_ENABLE_LOGGING
#ifdef WOOP_COLOR_LOGGING
  std::cout << "\x1b[31;1m";  // Bold red text
#endif
  std::cerr << "[woop:error] ";
  ((std::cerr << std::forward<Args>(args)), ...);
  std::cerr << std::endl;
#ifdef WOOP_COLOR_LOGGING
  std::cout << "\x1b[0m";
#endif
#else
  (UNUSED_PARAMETER(args), ...);
#endif
}

/**
 * @brief Logs a fatal error
 */
template <typename... Args>
inline constexpr void log_fatal(Args&&... args) {
#ifdef WOOP_ENABLE_LOGGING
#ifdef WOOP_COLOR_LOGGING
  std::cout << "\x1b[41;1;30m";  // Bold black text on red background
#endif
  std::cerr << "[woop:fatal] ";
  ((std::cerr << std::forward<Args>(args)), ...);
  std::cerr << std::endl;
#ifdef WOOP_COLOR_LOGGING
  std::cout << "\x1b[0m";
#endif
#else
  (UNUSED_PARAMETER(args), ...);
#endif
}
}  // namespace woop