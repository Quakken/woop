/**
 * @file bsp.hpp
 * @authors quak
 * @brief Declares types and utilities for creating and traversing BSP trees.
 */

#pragma once

#include "exception.hpp" /* woop::Exception */
#include "glm/glm.hpp"   /* glm::vec2, glm::dot  */
#include <variant>       /* std::variant, std::holds_alternative */

namespace woop {
struct Subsector;

/**
 * @brief Exception thrown when an error related to BSP traversal occurs.
 */
class BSPException : public Exception {
 public:
  /**
   * @brief Describes the type of error encountered.
   */
  enum class Type {
    InvalidNodeAccess,
  };

  BSPException(Type type, const std::string_view& what)
      : Exception(what), t(type) {}

  Type type() const noexcept { return t; }

 private:
  Type t;
};

/**
 * @brief Data for a single node of a level's BSP tree.
 */
class Node {
 public:
  enum class Child { Left, Right };

 public:
  Node(const glm::vec2& part_start, const glm::vec2& part_end);

  /**
   * @brief Returns which side of the partition a point is closest to.
   */
  Child get_nearest_child(const glm::vec2& point) const noexcept;

  /**
   * @brief Returns whether a child holds a node.
   */
  bool is_node(Child child) const noexcept;
  bool is_node_left() const noexcept { return is_node(Child::Left); }
  bool is_node_right() const noexcept { return is_node(Child::Right); }

  /**
   * @brief Returns whether a child holds a subsector.
   */
  bool is_subsector(Child child) const noexcept;
  bool is_subsector_left() const noexcept { return is_subsector(Child::Left); }
  bool is_subsector_right() const noexcept {
    return is_subsector(Child::Right);
  }

  /**
   * @brief Sets a child as a node.
   */
  void set_node(Node& node, Child child) noexcept;
  void set_node_left(Node& node) noexcept { set_node(node, Child::Left); }
  void set_node_right(Node& node) noexcept { set_node(node, Child::Right); }

  /**
   * @brief Returns the given child as a node.
   */
  const Node& get_node(Child child) const;
  Node& get_node(Child child);
  const Node& get_node_left() const { return get_node(Child::Left); }
  Node& get_node_left() { return get_node(Child::Left); }
  const Node& get_node_right() const { return get_node(Child::Right); }
  Node& get_node_right() { return get_node(Child::Right); }

  /**
   * @brief Sets an child as a subsector.
   */
  void set_subsector(Subsector& subsector, Child child) noexcept;
  void set_subsector_left(Subsector& subsector) noexcept {
    set_subsector(subsector, Child::Left);
  }
  void set_subsector_right(Subsector& subsector) noexcept {
    set_subsector(subsector, Child::Right);
  }

  /**
   * @brief Returns the given child as a subsector.
   */
  const Subsector& get_subsector(Child child) const;
  Subsector& get_subsector(Child child);
  const Subsector& get_subsector_left() const {
    return get_subsector(Child::Left);
  }
  Subsector& get_subsector_left() { return get_subsector(Child::Left); }
  const Subsector& get_subsector_right() const {
    return get_subsector(Child::Right);
  }
  Subsector& get_subsector_right() { return get_subsector(Child::Right); }

 private:
  std::variant<Subsector*, Node*> left_child;
  std::variant<Subsector*, Node*> right_child;
  glm::vec2 partition_start;
  glm::vec2 partition_end;
};
}  // namespace woop