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

/**
 * @brief Creates woop's primary window, based on configuration.
 */
woop::Window create_window() {
  woop::WindowConfig cfg{
      // Configuration options go here
  };
  return woop::Window{cfg};
}

/**
 * @brief Woop's main loop. Blocks until application is closed.
 */
void run_loop() {
  woop::Window window = create_window();

  while (!window.should_close()) {
    window.swap_buffers();
    glfwPollEvents();
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