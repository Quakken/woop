/**
 * @file window.cpp
 * @authors quak
 * @brief Defines members of the Window class.
 */

#include "window.hpp"
#include "GLFW/glfw3.h"
#include "log.hpp"
#include "utils.hpp"

namespace woop {
Window::Window(const WindowConfig& cfg) : config(cfg) {
  if (num_windows <= 0)
    init_glfw();
  init_window();
  if (!glad_initialized)
    init_glad();
  set_callbacks();
  ++num_windows;

  // Set viewport size
  glViewport(0, 0, config.size.x, config.size.y);
}
Window::~Window() {
  glfwDestroyWindow(window);
  --num_windows;
  if (num_windows <= 0)
    shutdown_glfw();
}
std::string Window::get_title() const noexcept {
  return glfwGetWindowTitle(window);
}
void Window::set_title(const std::string& new_title) noexcept {
  glfwSetWindowTitle(window, new_title.c_str());
}
glm::ivec2 Window::get_size() const noexcept {
  glm::ivec2 out;
  glfwGetWindowSize(window, &out.x, &out.y);
  return out;
}
void Window::set_size(const glm::ivec2& new_size) noexcept {
  glfwSetWindowSize(window, new_size.x, new_size.y);
}
float Window::get_aspect_ratio() const {
  float x = static_cast<float>(get_size().x);
  float y = static_cast<float>(get_size().y);
  if (y == 0)
    throw WindowException(WindowException::Type::Other,
                          "Window has no vertical size (is it minimized?)");
  return x / y;
}

void Window::init_glfw() {
  log("Initializing GLFW");
  glfwInit();
  glfwSetErrorCallback(error_callback);
}
void Window::init_glad() {
  log("Initializing GLAD");
  int result = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  if (!result)
    throw WindowException(WindowException::Type::GladInitialization,
                          "Unable to initialize GLAD");
  glad_initialized = true;
}
void Window::shutdown_glfw() {
  log("Shutting down GLFW");
  glfwTerminate();
}

void Window::init_window() {
  set_window_hints();
  GLFWmonitor* monitor = nullptr;
  if (config.fullscreen)
    monitor = glfwGetPrimaryMonitor();
  window = glfwCreateWindow(config.size.x, config.size.y, config.title.c_str(),
                            monitor, NULL);
  if (!window) {
    if (num_windows == 0)
      shutdown_glfw();
    throw WindowException(WindowException::Type::CreationError,
                          "Could not create window.");
  }
  glfwMakeContextCurrent(window);
}
void Window::set_window_hints() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  glfwWindowHint(GLFW_RESIZABLE, config.resizable);
  glfwWindowHint(GLFW_DECORATED, config.decorated);
}
void Window::set_callbacks() {
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

void Window::error_callback(int error, const char* description) noexcept {
  UNUSED_PARAMETER(error);
  log_error("GLFW error occured: ", description);
}
void Window::framebuffer_size_callback(GLFWwindow* window,
                                       int width,
                                       int height) noexcept {
  UNUSED_PARAMETER(window);
  glViewport(0, 0, width, height);
}

bool Window::should_close() const noexcept {
  return glfwWindowShouldClose(window);
}
void Window::swap_buffers() noexcept {
  glfwSwapBuffers(window);
}
void Window::close() noexcept {
  glfwSetWindowShouldClose(window, true);
}
void Window::minimize() noexcept {
  glfwIconifyWindow(window);
}
void Window::focus() noexcept {
  glfwFocusWindow(window);
}

}  // namespace woop