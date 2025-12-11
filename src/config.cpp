#include "config.hpp"
#include "level.hpp"

namespace config {
woop::Wad get_wad(const toml::table& table) {
  std::optional<std::string> wad_path =
      table["general"]["wad"].value<std::string>();
  if (!wad_path)
    throw ConfigException(
        "No WAD given. Specify a WAD to load with \"general.wad\"");
  return woop::Wad{*wad_path};
}
woop::Level get_level(const woop::Wad& wad, const toml::table& table) {
  std::optional<std::string> level_name =
      table["general"]["level"].value<std::string>();
  if (!level_name)
    throw ConfigException(
        "No level given. Specify a level with \"general.level\"");
  return woop::Level{wad, *level_name};
}

woop::WindowConfig get_window_config(const toml::table& table) {
  woop::WindowConfig cfg;
  if (const auto& entry = table["window"]["title"].value<std::string>())
    cfg.title = entry.value();
  if (const auto* entry = table["window"]["resolution"].as_array()) {
    // Resolution needs to be read in two parts
    cfg.resolution.x = entry->at(0).as_integer()->get();
    cfg.resolution.y = entry->at(1).as_integer()->get();
  }
  if (const auto& entry = table["window"]["fullscreen"].value<bool>())
    cfg.fullscreen = entry.value();
  if (const auto& entry = table["window"]["resizable"].value<bool>())
    cfg.resizable = entry.value();
  if (const auto& entry = table["window"]["decorated"].value<bool>())
    cfg.decorated = entry.value();
  return cfg;
}
woop::CameraConfig get_camera_config(const toml::table& table) {
  woop::CameraConfig cfg;
  if (const auto& entry = table["camera"]["near_plane"].value<double>())
    cfg.near_plane = entry.value();
  if (const auto& entry = table["camera"]["far_plane"].value<double>())
    cfg.far_plane = entry.value();
  if (const auto& entry = table["camera"]["fov"].value<double>())
    cfg.fov = entry.value();
  return cfg;
}
woop::RendererConfig get_renderer_config(const toml::table& table) {
  woop::RendererConfig cfg;
  if (const auto& entry =
          table["renderer"]["vertex_shader"].value<std::string>())
    cfg.shaders.vert_path = entry.value();
  if (const auto& entry =
          table["renderer"]["fragment_shader"].value<std::string>())
    cfg.shaders.frag_path = entry.value();
  if (const auto* entry = table["renderer"]["clear_color"].as_array()) {
    try {
      cfg.clear_color.r =
          static_cast<std::byte>(entry->at(0).as_integer()->get());
      cfg.clear_color.g =
          static_cast<std::byte>(entry->at(1).as_integer()->get());
      cfg.clear_color.b =
          static_cast<std::byte>(entry->at(2).as_integer()->get());
    } catch (std::exception& except) {
      throw ConfigException(
          "Invalid value given to \"renderer.clear_color\" (expected array of "
          "3 integers)");
    }
  }
  if (const auto* entry = table["renderer"]["fog_color"].as_array()) {
    try {
      cfg.fog_color.r =
          static_cast<std::byte>(entry->at(0).as_integer()->get());
      cfg.fog_color.g =
          static_cast<std::byte>(entry->at(1).as_integer()->get());
      cfg.fog_color.b =
          static_cast<std::byte>(entry->at(2).as_integer()->get());
    } catch (std::exception& except) {
      throw ConfigException(
          "Invalid value given to \"renderer.fog_color\" (expected array of "
          "3 integers)");
    }
  }
  if (const auto& entry = table["renderer"]["fog_strength"].value<double>())
    cfg.fog_strength = entry.value();
  return cfg;
}
woop::PlayerConfig get_player_config(const toml::table& table) {
  woop::PlayerConfig cfg;
  if (const auto& entry = table["player"]["height"].value<double>())
    cfg.camera_height = entry.value();
  if (const auto& entry = table["player"]["gravity"].value<double>())
    cfg.gravity = entry.value();
  if (const auto& entry = table["player"]["sensitivity"].value<double>())
    cfg.sensitivity = entry.value();
  if (const auto& entry = table["player"]["move_speed"].value<double>())
    cfg.move_speed = entry.value();
  if (const auto& entry = table["player"]["acceleration"].value<double>())
    cfg.acceleration = entry.value();
  if (const auto& entry = table["player"]["drag"].value<double>())
    cfg.drag = entry.value();
  if (const auto& entry = table["player"]["enable_mouse"].value<bool>())
    cfg.enable_mouse = entry.value();
  if (const auto& entry = table["player"]["enable_flight"].value<bool>())
    cfg.enable_flight = entry.value();
  return cfg;
}
}  // namespace config