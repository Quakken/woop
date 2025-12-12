/**
 * @file main.cpp
 * @authors quak
 * @brief Defines the engine's entry point, initializing the engine, reading
 * config data, and starting the simulation.
 */

#include "exception.hpp"   /* woop::Exception */
#include "level.hpp"       /* woop::Level */
#include "log.hpp"         /* woop::log, woop::log_error, woop::log_fatal */
#include "utils.hpp"       /* UNUSED_PARAMETER */
#include "window.hpp"      /* woop::Window */
#include "camera.hpp"      /* woop::Camera */
#include "renderer.hpp"    /* woop::Renderer */
#include "player.hpp"      /* woop::Player */
#include "wad.hpp"         /* woop::Wad */
#include "config.hpp"      /* Configuration parsing */
#include "toml++/toml.hpp" /* toml::table  */

woop::Window create_window(const toml::table& table) {
  woop::WindowConfig cfg = config::get_window_config(table);
  return woop::Window{cfg};
}

woop::Camera create_camera(woop::Window& window, const toml::table& table) {
  woop::CameraConfig cfg = config::get_camera_config(table);
  return woop::Camera{window, cfg};
}

woop::Renderer create_renderer(woop::Window& window,
                               woop::Camera& camera,
                               const toml::table& table) {
  woop::RendererConfig cfg = config::get_renderer_config(table);
  return woop::Renderer(window, camera, cfg);
}

woop::Player create_player(woop::Camera& camera,
                           const woop::Level& level,
                           const toml::table& table) {
  woop::PlayerConfig cfg = config::get_player_config(table);
  return woop::Player{camera, level, cfg};
}

void update_shader_properties(woop::Renderer& renderer) {
  woop::Shader& shader = renderer.get_shader();
  shader.set_uniform("u_time", static_cast<float>(glfwGetTime()));
}

/**
 * @brief Woop's main loop. Blocks until application is closed.
 */
void run_loop() {
  toml::table table = toml::parse_file("config.toml");

  /* Renderer data */
  woop::Window window = create_window(table);
  woop::Camera camera = create_camera(window, table);
  woop::Renderer renderer = create_renderer(window, camera, table);

  /* Level data */
  woop::Wad wad = config::get_wad(table);
  woop::Level level = config::get_level(wad, table);

  /* Player */
  woop::Player player = create_player(camera, level, table);

  float time = glfwGetTime();
  while (!window.should_close()) {
    /* Calculate time since last update */
    float dt = glfwGetTime() - time;
    time = glfwGetTime();
    glfwPollEvents();

    /* Move player */
    player.update(dt);

    /* Update shader uniforms */
    update_shader_properties(renderer);

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
  } catch (config::ConfigException& exception) {
    woop::log_fatal("Configuration error: ", exception.what());
  } catch (woop::Exception& exception) {
    woop::log_fatal("Exception caught: ", exception.what());
  } catch (toml::parse_error& exception) {
    woop::log_fatal("Error parsing configuration (", exception.source(),
                    "): ", exception.description());
  } catch (std::exception& exception) {
    woop::log_fatal("Standard exception caught: ", exception.what());
  }
}