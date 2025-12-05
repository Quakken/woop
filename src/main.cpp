/**
 * @file main.cpp
 * @authors quak
 * @brief Defines the engine's entry point, initializing the engine, reading
 * config data, and starting the simulation.
 */

#include "exception.hpp" /* woop::Exception */
#include "glm/fwd.hpp"
#include "level.hpp"
#include "log.hpp"      /* woop::log, woop::log_error, woop::log_fatal */
#include "utils.hpp"    /* UNUSED_PARAMETER */
#include "window.hpp"   /* woop::Window */
#include "camera.hpp"   /* woop::Camera */
#include "renderer.hpp" /* woop::Renderer */
#include "player.hpp"   /* woop::Player */
#include "wad.hpp"      /* woop::Wad */

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
      .position = {3000.0f, 0.0f, -4800.0f},
      .fov = 60.0f,
  };
  return woop::Camera{window, cfg};
}

woop::Renderer create_renderer(woop::Window& window, woop::Camera& camera) {
  woop::RendererConfig cfg = {
      // Configuration options go here
      .clear_color = woop::Pixel{27, 27, 37, 255},
      .fill_color = woop::Pixel{215, 212, 206, 255},
  };
  return woop::Renderer(window, camera, cfg);
}

woop::Player create_player(woop::Camera& camera, const woop::Level& level) {
  woop::PlayerConfig cfg = {
      // Configuration options go here
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
  woop::Level e1m1 = {doom1, "E1M1"};

  /* Player */
  woop::Player player = create_player(camera, e1m1);

  /* TEMP: Set camera position to player 1 start */
  woop::Lump things_lump = doom1.get_lump("E1M2", "THINGS");
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
    /* Calculate time since last update */
    float dt = glfwGetTime() - time;
    time = glfwGetTime();
    glfwPollEvents();

    /* Move player */
    player.update(dt);

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