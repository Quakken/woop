/**
 * @file player.hpp
 * @authors quak
 * @brief Defines the Player class, which can be used to move a camera through a
 * level with first-person controls.
 */

#pragma once

#include "camera.hpp"   /* woop::Camera */
#include "level.hpp"    /* woop::Level */
#include "glm/vec2.hpp" /* glm::vec2 */
#include "GLFW/glfw3.h" /* GLFWwindow */

namespace woop {
/**
 * @broef Configuration data for the player controller.
 */
struct PlayerConfig {
  float camera_height = 30.0f;
  float gravity = 9.8f * 100.0f;
  float sensitivity = 1.0f;
  float move_speed = 650.0f;
  float acceleration = 3 * move_speed;
  float drag = 0.1f;
  bool enable_mouse = true;
  bool enable_flight = false;
};

/**
 * @brief Allows for a camera to be moved around a level with first-person
 * controls.
 */
class Player {
 public:
  Player(Camera& camera, const Level& level, const PlayerConfig& config);

  /**
   * @brief Updates the player's position and rotation.
   * @param dt Time since last update.
   */
  void update(float dt);

  /**
   * @brief Returns a reference to the associated camera.
   */
  const Camera& get_camera() const noexcept { return camera; }
  Camera& get_camera() noexcept { return camera; }

  /**
   * @brief Moves the player to a new level.
   */
  void set_level(const Level& level) noexcept;
  /**
   * @brief Returns a reference to the player's current level.
   */
  const Level& get_level() const noexcept { return *level; }

  /**
   * @brief Returns a reference to the subsector that the player is closest to.
   */
  const Subsector& get_current_subsector() noexcept;

 private:
  static void key_callback(GLFWwindow* window,
                           int key,
                           int scancode,
                           int action,
                           int mods);
  static void cursor_position_callback(GLFWwindow* window,
                                       double xpos,
                                       double ypos);

  void update_position(float dt);
  void update_rotation();
  void update_current_subsector();

  glm::vec2 get_direction() const noexcept;
  void do_horizontal_accel(const glm::vec2& dir, float dt);
  void do_flight(float dt);
  void do_gravity(float dt);

  float get_floor_height();

  inline static glm::vec3 input;
  inline static float mouse_delta;

  PlayerConfig config;
  const Subsector* current_subsector;
  const Level* level;
  Camera& camera;
  glm::vec2 horiz_vel;
  float vert_vel;
  bool is_subsector_dirty;
};
}  // namespace woop