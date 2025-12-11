/**
 * @file config.hpp
 * @authors quak
 * @brief Declares a handful of functions to generate Woop configurations from
 * a toml file.
 */

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
/**
 * @brief Exception thrown when a configuration-related error occurs.
 */
struct ConfigException : public woop::Exception {
 public:
  ConfigException(const std::string_view& what) : woop::Exception(what) {}
};

/**
 * @brief Returns the wad specified in the renderer's configuration file.
 */
woop::Wad get_wad(const toml::table& table);
/**
 * @brief Returns the level of the wad specified in the renderer's configuration
 * file.
 */
woop::Level get_level(const woop::Wad& wad, const toml::table& table);

/**
 * @brief Fills a window configuration with values present in a configuration
 * file.
 */
woop::WindowConfig get_window_config(const toml::table& table);
/**
 * @brief Fills a camera configuration with values present in a configuration
 * file.
 */
woop::CameraConfig get_camera_config(const toml::table& table);
/**
 * @brief Fills a renderer configuration with values present in a configuration
 * file.
 */
woop::RendererConfig get_renderer_config(const toml::table& table);
/**
 * @brief Fills a player configuration with values present in a configuration
 * file.
 */
woop::PlayerConfig get_player_config(const toml::table& table);
}  // namespace config