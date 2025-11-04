/**
 * @file exception.hpp
 * @authors quak
 * @brief Defines woop's Exception class, which all woop exceptions derive from.
 */

#pragma once

#include <string_view>
#include <exception>

namespace woop {
/**
 * @brief Base class that all woop exceptions derive from.
 */
class Exception : public std::exception {
 public:
  Exception(const std::string_view& what) : msg(what) {}

  const char* what() const noexcept override { return msg.data(); }

 private:
  std::string_view msg;
};
}  // namespace woop