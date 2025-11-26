/**
 * @file main.cpp
 * @authors quak
 * @brief Defines the engine's entry point, initializing the engine, reading
 * config data, and starting the simulation.
 */

#include "exception.hpp" /* woop::Exception */
#include "glm/fwd.hpp"
#include "log.hpp"      /* woop::log, woop::log_error, woop::log_fatal */
#include "utils.hpp"    /* UNUSED_PARAMETER */
#include "window.hpp"   /* woop::Window */
#include "camera.hpp"   /* woop::Camera */
#include "renderer.hpp" /* woop::Renderer */
#include "wad.hpp"      /* woop::Wad */

/**
 * @brief Creates woop's primary window, based on configuration.
 */
woop::Window create_window() {
  woop::WindowConfig cfg = {
      // Configuration options go here
      .resolution = {1280 / 3, 720 / 3},
      .decorated = true,
  };
  return woop::Window{cfg};
}

woop::Camera create_camera(const woop::Window& window) {
  woop::CameraConfig cfg = {
      // Configuration options go here
      .position = {3000.0f, 0.0f, -4800.0f},
      .fov = 60.0f,
  };
  return woop::Camera{window, cfg};
}

woop::Renderer create_renderer(woop::Window& window, woop::Camera& camera) {
  woop::RendererConfig cfg = {
      // Configuration options go here
      .clear_color = woop::Pixel{125, 125, 125, 255},
  };
  return woop::Renderer(window, camera, cfg);
}

void move_camera(woop::Window& window, woop::Camera& cam) {
  static glm::dvec2 prev_cursor_pos{};
  glm::vec2 input{};
  if (glfwGetKey(window.get_wrapped(), GLFW_KEY_W) == GLFW_PRESS)
    input.y++;
  if (glfwGetKey(window.get_wrapped(), GLFW_KEY_S) == GLFW_PRESS)
    input.y--;
  if (glfwGetKey(window.get_wrapped(), GLFW_KEY_D) == GLFW_PRESS)
    input.x++;
  if (glfwGetKey(window.get_wrapped(), GLFW_KEY_A) == GLFW_PRESS)
    input.x--;
  glm::dvec2 current_cursor_pos;
  glfwGetCursorPos(window.get_wrapped(), &current_cursor_pos.x,
                   &current_cursor_pos.y);
  glm::dvec2 cursor_delta = current_cursor_pos - prev_cursor_pos;
  cam.set_rotation(cam.get_rotation() + cursor_delta.x * 1.0f);
  cam.set_position(cam.get_position() +
                   glm::vec3{
                       -input.x * sin(glm::radians(cam.get_rotation())) -
                           -input.y * cos(glm::radians(cam.get_rotation())),
                       0.0f,
                       -input.x * cos(glm::radians(cam.get_rotation())) +
                           -input.y * sin(glm::radians(cam.get_rotation())),
                   });
  prev_cursor_pos = current_cursor_pos;
}

struct Thing {
  int16_t x;
  int16_t y;
  int16_t angle;
  int16_t type;
  int16_t flags;
};

/**
 * @brief Woop's main loop. Blocks until application is closed.
 */
void run_loop() {
  /* Renderer data */
  woop::Window window = create_window();
  woop::Camera camera = create_camera(window);
  woop::Renderer renderer = create_renderer(window, camera);

  /* Level data */
  woop::Wad doom1 = {"assets/wads/doom1.wad"};
  woop::Level e1m1 = {doom1, "E1M1"};

  woop::Lump things_lump = doom1.get_lump("E1M1", "THINGS");
  std::vector<Thing> things = things_lump.get_data_as<Thing>();
  for (const auto& thing : things) {
    if (thing.type == 1) {
      camera.set_position(glm::vec3{thing.x, 35.0f, thing.y});
      camera.set_rotation(doom_angle_to_deg(thing.angle) - 90);
      break;
    }
  }

  float time = glfwGetTime();
  while (!window.should_close()) {
    glfwPollEvents();
    // camera.set_rotation(camera.get_rotation() + (glfwGetTime() - time) *
    // 360);
    move_camera(window, camera);
    woop::Frame frame = renderer.begin_frame();
    frame.draw(woop::DrawMode::Solid, e1m1.get_root_node());
    time = glfwGetTime();
  }
}

int main(int argc, const char* argv[]) {
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);

  try {
    run_loop();
  } catch (woop::Exception& exception) {
    woop::log_fatal("Exception caught: ", exception.what());
  } catch (std::exception& exception) {
    woop::log_fatal("Standard exception caught: ", exception.what());
  }
}