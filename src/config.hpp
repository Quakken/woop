#pragma once

#include "window.hpp"      /* woop::Window */
#include "camera.hpp"      /* woop::Camera */
#include "renderer.hpp"    /* woop::Renderer */
#include "player.hpp"      /* woop::Player */
#include "exception.hpp"   /* woop::Exception */
#include "wad.hpp"         /* woop::Wad */
#include "level.hpp"       /* woop::Level */
#include "toml++/toml.hpp" /* Configuration parsing */

namespace config {
struct ConfigException : public woop::Exception {
 public:
  ConfigException(const std::string_view& what) : woop::Exception(what) {}
};

woop::Wad get_wad(const toml::table& table);
woop::Level get_level(const woop::Wad& wad, const toml::table& table);

woop::WindowConfig get_window_config(const toml::table& table);
woop::CameraConfig get_camera_config(const toml::table& table);
woop::RendererConfig get_renderer_config(const toml::table& table);
woop::PlayerConfig get_player_config(const toml::table& table);
}  // namespace config