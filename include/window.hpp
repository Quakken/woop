/**
 * @file window.cpp
 * @authors quak
 * @brief Declares the Window class, which can be used to create system windows.
 */

#pragma once

#include "exception.hpp" /* woop::Exception */
#include "glad/glad.h"   /* GLAD function loading */
#include "GLFW/glfw3.h"  /* GLFW windows and events */
#include "glm/vec2.hpp"  /* glm::ivec2 */
#include <string>        /* std::string */

namespace woop {
/**
 * @brief Exception thrown when a Window encounters an error.
 */
struct WindowException : public Exception {
 public:
  /**
   * @brief Describes the type of error encountered.
   */
  enum class Type {
    CreationError,
    GladInitialization,
    GLFWInitialization,
  };

  WindowException(Type type, const std::string_view& what)
      : Exception(what), t(type) {}
  Type type() const noexcept { return t; }

 private:
  Type t;
};

/**
 * @brief Configuration data for a single window.
 */
struct WindowConfig {
  std::string title = "Woop";
  glm::ivec2 resolution = {1280, 720};
  bool fullscreen = false;
  bool resizable = true;
  bool decorated = true;
};

/**
 * @brief Represents a system-level window.
 */
class Window {
 public:
  Window(const WindowConfig& cfg = WindowConfig{});
  Window(const Window&) = delete;
  ~Window();

  Window& operator=(const Window&) = delete;

  void swap_buffers() noexcept;
  void close() noexcept;
  void minimize() noexcept;
  void focus() noexcept;

  bool should_close() const noexcept;

  std::string get_title() const noexcept;
  void set_title(const std::string& new_title) noexcept;

  glm::ivec2 get_resolution() const noexcept;
  void set_resolution(const glm::ivec2& new_resolution) noexcept;

  /**
   * @brief Returns a reference to the underlying GLFWwindow object.
   */
  const GLFWwindow* get_wrapped() const noexcept { return window; }
  GLFWwindow* get_wrapped() noexcept { return window; }

 private:
  static void init_glfw();
  static void init_glad();
  static void shutdown_glfw();

  void init_window();
  void set_window_hints();
  void set_callbacks();

  static void error_callback(int error, const char* description) noexcept;
  static void framebuffer_size_callback(GLFWwindow* window,
                                        int width,
                                        int height) noexcept;

  inline static unsigned num_windows = 0;
  inline static bool glad_initialized = false;

  GLFWwindow* window;
  WindowConfig config;
};
}  // namespace woop