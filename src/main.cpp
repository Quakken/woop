/**
 * @file main.cpp
 * @authors quak
 * @brief Defines the engine's entry point, initializing the engine, reading
 * config data, and starting the simulation.
 */

#include "exception.hpp" /* woop::Exception */
#include "level.hpp"     /* woop::Level */
#include "log.hpp"       /* woop::log, woop::log_error, woop::log_fatal */
#include "utils.hpp"     /* UNUSED_PARAMETER */
#include "window.hpp"    /* woop::Window */
#include "camera.hpp"    /* woop::Camera */
#include "renderer.hpp"  /* woop::Renderer */
#include "player.hpp"    /* woop::Player */
#include "wad.hpp"       /* woop::Wad */

/**
 * @brief Creates woop's primary window, based on configuration.
 */
woop::Window create_window() {
  woop::WindowConfig cfg = {
      // Configuration options go here
      .resolution = {320, 200},
      .decorated = true,
  };
  return woop::Window{cfg};
}

woop::Camera create_camera(woop::Window& window) {
  woop::CameraConfig cfg = {
      // Configuration options go here
      .fov = 90.0f,
  };
  return woop::Camera{window, cfg};
}

woop::Renderer create_renderer(woop::Window& window, woop::Camera& camera) {
  woop::RendererConfig cfg = {
      // Configuration options go here
      .clear_color = woop::Pixel{27, 27, 37, 255},
      .fill_color = woop::Pixel{215, 212, 206, 255},
      .fog_strength = 0.75f,
  };
  return woop::Renderer(window, camera, cfg);
}

woop::Player create_player(woop::Camera& camera, const woop::Level& level) {
  woop::PlayerConfig cfg = {
      // Configuration options go here
      .camera_height = 45.0f,
      .move_speed = 300.0f,
      .enable_flight = false,
  };
  return woop::Player{camera, level, cfg};
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
  woop::Level levels[] = {
      woop::Level{doom1, "E1M1"},
      woop::Level{doom1, "E1M2"},
      woop::Level{doom1, "E1M3"},
      woop::Level{doom1, "E1M4"},
  };

  /* Player */
  woop::Player player = create_player(camera, levels[0]);

  float time = glfwGetTime();
  while (!window.should_close()) {
    /* Calculate time since last update */
    float dt = glfwGetTime() - time;
    time = glfwGetTime();
    glfwPollEvents();

    /* Move player */
    player.update(dt);

    /* DEMO: Changing levels */
    for (int i = 0; i < sizeof(levels) / sizeof(woop::Level); ++i) {
      if (glfwGetKey(window.get_wrapped(), GLFW_KEY_1 + i))
        player.set_level(levels[i]);
    }

    /* Draw level */
    woop::Frame frame = renderer.begin_frame();
    frame.draw(woop::DrawMode::Solid, player.get_level().get_root_node());
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