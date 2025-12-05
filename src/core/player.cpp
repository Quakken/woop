/**
 * @file player.cpp
 * @authors quak
 * @brief Defines members of the Player class.
 */

#include "player.hpp"
#include <stdexcept>
#include "GLFW/glfw3.h"
#include "glm/geometric.hpp"
#include "level.hpp"
#include "utils.hpp"

namespace woop {
// "Type" of the Player 1 start Thing (https://doomwiki.org/wiki/Thing_typesq)
constexpr int16_t player_start_thing = 1;

Player::Player(Camera& cam, const Level& lvl, const PlayerConfig& conf)
    : config(conf), camera(cam), horiz_vel(0.0f), vert_vel(0.0f) {
  GLFWwindow* window = camera.get_window().get_wrapped();
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  set_level(lvl);
  glfwSetWindowFocusCallback(window, window_focus_callback);
  // Disable cursor
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Player::update(float dt) {
  update_position(dt);
  update_rotation();
}

void Player::set_level(const Level& lvl) noexcept {
  level = &lvl;
  is_subsector_dirty = true;

  // Search for player start position
  const std::vector<Thing>& things = level->get_things();
  for (const auto& thing : things) {
    if (thing.type == player_start_thing) {
      camera.set_position(glm::vec3{
          thing.position.x,
          config.camera_height,
          thing.position.y,
      });
      camera.set_rotation(thing.angle - 90.0f);
    }
  }
}

const Subsector& Player::get_current_subsector() noexcept {
  if (is_subsector_dirty)
    update_current_subsector();
  return *current_subsector;
}

void Player::update_current_subsector() {
  const Node* node = &level->get_root_node();
  Node::Child nearest_child = node->get_nearest_child(camera.get_position_2d());
  while (node->is_node(nearest_child)) {
    node = &node->get_node(nearest_child);
    nearest_child = node->get_nearest_child(camera.get_position_2d());
  }
  current_subsector = &node->get_subsector(nearest_child);
  is_subsector_dirty = false;
}

void Player::update_position(float dt) {
  glm::vec2 direction = get_direction();

  do_horizontal_accel(direction, dt);
  if (config.enable_flight)
    do_flight(dt);
  do_gravity(dt);

  if (horiz_vel.x != 0.0f || horiz_vel.y != 0.0f || vert_vel != 0.0f) {
    glm::vec3 velocity = {
        horiz_vel.x,
        vert_vel,
        horiz_vel.y,
    };
    camera.set_position(camera.get_position() + velocity * dt);
    is_subsector_dirty = true;
  }
}

glm::vec2 Player::get_direction() const noexcept {
  if (config.enable_mouse) {
    // X input used for movement
    return {
        -input.x * sin(glm::radians(camera.get_rotation())) +
            input.z * cos(glm::radians(camera.get_rotation())),
        -input.x * cos(glm::radians(camera.get_rotation())) -
            input.z * sin(glm::radians(camera.get_rotation())),
    };
  }
  // X input used for rotation
  return {
      input.z * cos(glm::radians(camera.get_rotation())),
      -input.z * sin(glm::radians(camera.get_rotation())),
  };
}

void Player::do_horizontal_accel(const glm::vec2& dir, float dt) {
  if (dir.x != 0 || dir.y != 0) {
    // Correct direction
    if (glm::dot(horiz_vel, dir) != 0.0f)
      horiz_vel = glm::normalize(dir) * glm::length(horiz_vel);

    glm::vec2 horiz_accel = config.acceleration * glm::normalize(dir);
    horiz_vel += horiz_accel * dt;
  }
  // Drag
  else
    horiz_vel *= std::clamp(1.0f - config.drag, 0.0f, 1.0f);

  // Limit horizontal speed
  if (glm::length(horiz_vel) > config.move_speed)
    horiz_vel = glm::normalize(horiz_vel) * config.move_speed;
}

void Player::do_flight(float dt) {
  if (input.y != 0.0f) {
    vert_vel += input.y * config.acceleration * dt;
    vert_vel = std::clamp(vert_vel, -config.move_speed, config.move_speed);

  } else
    vert_vel *= std::clamp(1.0f - config.drag, 0.0f, 1.0f);
}

void Player::do_gravity(float dt) {
  if (camera.get_position().y > get_floor_height() + config.camera_height) {
    if (!config.enable_flight)
      vert_vel -= config.gravity * dt;
  } else {
    if (!config.enable_flight || input.y <= 0)
      vert_vel = 0.0f;
    camera.set_position(glm::vec3{
        camera.get_position().x,
        get_floor_height() + config.camera_height,
        camera.get_position().z,
    });
  }
}

void Player::update_rotation() {
  float rotation = camera.get_rotation();
  if (config.enable_mouse) {
    rotation += mouse_delta * config.sensitivity;
    // FIXME: cursor_position_callback doesn't update mouse_delta when the
    // cursor doesn't move (mouse_delta is never 0).
    mouse_delta = 0.0f;
  } else {
    rotation += input.x * config.sensitivity;
  }
  while (rotation > 360)
    rotation -= 360.0f;
  camera.set_rotation(rotation);
}

void Player::key_callback(GLFWwindow* window,
                          int key,
                          int scancode,
                          int action,
                          int mods) {
  UNUSED_PARAMETER(window);
  UNUSED_PARAMETER(scancode);
  UNUSED_PARAMETER(mods);
  if (key == GLFW_KEY_W) {
    if (action == GLFW_PRESS)
      ++input.z;
    else if (action == GLFW_RELEASE)
      --input.z;
  }
  if (key == GLFW_KEY_S) {
    if (action == GLFW_PRESS)
      --input.z;
    else if (action == GLFW_RELEASE)
      ++input.z;
  }
  if (key == GLFW_KEY_D) {
    if (action == GLFW_PRESS)
      ++input.x;
    else if (action == GLFW_RELEASE)
      --input.x;
  }
  if (key == GLFW_KEY_A) {
    if (action == GLFW_PRESS)
      --input.x;
    else if (action == GLFW_RELEASE)
      ++input.x;
  }
  if (key == GLFW_KEY_SPACE) {
    if (action == GLFW_PRESS)
      ++input.y;
    else if (action == GLFW_RELEASE)
      --input.y;
  }
  if (key == GLFW_KEY_LEFT_CONTROL) {
    if (action == GLFW_PRESS)
      --input.y;
    else if (action == GLFW_RELEASE)
      ++input.y;
  }
}

void Player::cursor_position_callback(GLFWwindow* window,
                                      double x_pos,
                                      double y_pos) {
  UNUSED_PARAMETER(window);
  UNUSED_PARAMETER(y_pos);
  static float cursor_prev = static_cast<float>(x_pos);
  float cursor_current = static_cast<float>(x_pos);
  mouse_delta = cursor_current - cursor_prev;
  cursor_prev = cursor_current;
}

void Player::window_focus_callback(GLFWwindow* window, int focused) {
  // Show cursor when focus is lost
  if (focused)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  else
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

float Player::get_floor_height() {
  const Subsector& subsector = get_current_subsector();
  if (subsector.segs.empty())
    throw std::runtime_error("No segs in sector.");

  for (const Seg* seg : subsector.segs) {
    const Sidedef* sidedef = seg->sidedef;
    if (sidedef)
      return sidedef->sector_facing.floor.height;
  }
  return 0.0f;
}
}  // namespace woop