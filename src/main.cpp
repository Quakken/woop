/**
 * @file main.cpp
 * @authors quak
 * @brief Defines the engine's entry point, initializing the engine, reading
 * config data, and starting the simulation.
 */

#include "exception.hpp" /* woop::Exception */
#include "log.hpp"       /* woop::log, woop::log_error, woop::log_fatal */
#include "utils.hpp"     /* UNUSED_PARAMETER */
#include "window.hpp"    /* woop::Window */
#include "camera.hpp"    /* woop::Camera */
#include "renderer.hpp"  /* woop::Renderer */
#include "wad.hpp"       /* woop::Wad */

/**
 * @brief Creates woop's primary window, based on configuration.
 */
woop::Window create_window() {
  woop::WindowConfig cfg = {
      // Configuration options go here
      .decorated = false,
  };
  return woop::Window{cfg};
}

woop::Camera create_camera(const woop::Window& window) {
  woop::CameraConfig cfg = {
      // Configuration options go here
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

  while (!window.should_close()) {
    glfwPollEvents();
    woop::Frame frame = renderer.begin_frame();
    frame.draw(woop::DrawMode::Solid, e1m1.get_root_node());
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