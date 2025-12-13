/**
 * @file config.hpp
 * @authors quak
 * @brief Defines functions declared in config.hpp
 */

#include "config.hpp"
#include "level.hpp"
#include "renderer.hpp"
#include "wad.hpp"

namespace config {
glm::ivec2 get_array_as_ivec2(const toml::array& value) {
  return glm::ivec2{
      value.at(0).as_integer()->get(),
      value.at(1).as_integer()->get(),
  };
}
woop::Pixel get_array_as_pixel(const toml::array& value) {
  return woop::Pixel{
      value.at(0).as_integer()->get(),
      value.at(1).as_integer()->get(),
      value.at(2).as_integer()->get(),
      255,
  };
}

woop::Wad get_wad(const toml::table& table) {
  std::optional<std::string> wad_path =
      table["general"]["wad"].value<std::string>();
  if (!wad_path)
    throw ConfigException(
        "No WAD given. Specify a WAD to load with \"general.wad\"");
  try {
    return woop::Wad{*wad_path};
  } catch (woop::Exception& exception) {
    throw ConfigException(exception.what());
  }
}
woop::Level get_level(const woop::Wad& wad, const toml::table& table) {
  std::optional<std::string> level_name =
      table["general"]["level"].value<std::string>();
  if (!level_name)
    throw ConfigException(
        "No level given. Specify a level with \"general.level\"");
  try {
    return woop::Level{wad, *level_name};
  } catch (woop::Exception& exception) {
    throw ConfigException(exception.what());
  }
}

woop::WindowConfig get_window_config(const toml::table& table) {
  woop::WindowConfig cfg;
  // Title
  if (const auto& entry = table["window"]["title"].value<std::string>())
    cfg.title = entry.value();
  // Resolution
  if (const auto* entry = table["window"]["size"].as_array()) {
    try {
      cfg.size = get_array_as_ivec2(*entry);
    } catch (std::exception& exception) {
      throw ConfigException(
          "Invalid value given to \"window.size\" (expected [width, "
          "height])");
    }
  }
  // Fullscreen
  if (const auto& entry = table["window"]["fullscreen"].value<bool>())
    cfg.fullscreen = entry.value();
  // Resizable
  if (const auto& entry = table["window"]["resizable"].value<bool>())
    cfg.resizable = entry.value();
  // Decorated
  if (const auto& entry = table["window"]["decorated"].value<bool>())
    cfg.decorated = entry.value();
  return cfg;
}
woop::CameraConfig get_camera_config(const toml::table& table) {
  woop::CameraConfig cfg;
  // Near plane
  if (const auto& entry = table["camera"]["near_plane"].value<double>())
    cfg.near_plane = entry.value();
  // Far plane
  if (const auto& entry = table["camera"]["far_plane"].value<double>())
    cfg.far_plane = entry.value();
  // FOV
  if (const auto& entry = table["camera"]["fov"].value<double>())
    cfg.fov = entry.value();
  return cfg;
}
woop::RendererConfig get_renderer_config(const toml::table& table) {
  woop::RendererConfig cfg;
  // Vertex shader
  if (const auto& entry =
          table["renderer"]["vertex_shader"].value<std::string>())
    cfg.shaders.vert_path = entry.value();
  // Fragment shader
  if (const auto& entry =
          table["renderer"]["fragment_shader"].value<std::string>())
    cfg.shaders.frag_path = entry.value();
  // Resolution
  if (const auto* entry = table["renderer"]["resolution"].as_array()) {
    glm::ivec2 resolution;
    try {
      resolution = get_array_as_ivec2(*entry);
    } catch (std::exception& exception) {
      throw ConfigException(
          "Invalid value given to \"renderer.resolution\" (expected [width, "
          "height])");
    }
    if (resolution.x < 0 || resolution.y < 0)
      throw ConfigException("Renderer resolution must be positive.");
    cfg.resolution = resolution;
  }
  // Clear color
  if (const auto* entry = table["renderer"]["clear_color"].as_array()) {
    try {
      cfg.clear_color = get_array_as_pixel(*entry);
    } catch (std::exception& except) {
      throw ConfigException(
          "Invalid value given to \"renderer.clear_color\" (expected [r, g, "
          "b])");
    }
  }
  // Fog color
  if (const auto* entry = table["renderer"]["fog_color"].as_array()) {
    try {
      cfg.fog_color = get_array_as_pixel(*entry);
    } catch (std::exception& except) {
      throw ConfigException(
          "Invalid value given to \"renderer.fog_color\" (expected [r, g, "
          "b])");
    }
  }
  // Fog strength
  if (const auto& entry = table["renderer"]["fog_strength"].value<double>())
    cfg.fog_strength = entry.value();
  return cfg;
}
woop::PlayerConfig get_player_config(const toml::table& table) {
  woop::PlayerConfig cfg;
  // Height
  if (const auto& entry = table["player"]["height"].value<double>())
    cfg.camera_height = entry.value();
  // Gravity
  if (const auto& entry = table["player"]["gravity"].value<double>())
    cfg.gravity = entry.value();
  // Sensitivity
  if (const auto& entry = table["player"]["sensitivity"].value<double>())
    cfg.sensitivity = entry.value();
  // Move speed
  if (const auto& entry = table["player"]["move_speed"].value<double>())
    cfg.move_speed = entry.value();
  // Acceleration
  if (const auto& entry = table["player"]["acceleration"].value<double>())
    cfg.acceleration = entry.value();
  // Drag
  if (const auto& entry = table["player"]["drag"].value<double>())
    cfg.drag = entry.value();
  // Mouse rotation
  if (const auto& entry = table["player"]["enable_mouse"].value<bool>())
    cfg.enable_mouse = entry.value();
  // Flight
  if (const auto& entry = table["player"]["enable_flight"].value<bool>())
    cfg.enable_flight = entry.value();
  return cfg;
}
}  // namespace config