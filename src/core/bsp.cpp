/**
 * @file bsp.cpp
 * @authors quak
 * @brief Defines members of the Node class
 */

#include "bsp.hpp"
#include "level.hpp" /* woop::Sector */

namespace woop {
Node::Node(const glm::vec2& part_start, const glm::vec2& part_end)
    : partition_start(part_start), partition_end(part_end) {}

Node::Child Node::get_nearest_child(const glm::vec2& point) const noexcept {
  // glm::vec2 part_dir = partition_end - partition_start;
  // float slope = part_dir.y / part_dir.x;
  // float result =
  //     slope * (partition_start.x - point.x) + point.y - partition_start.y;
  float result =
      (point.x - partition_start.x) * (partition_end.y - partition_start.y) -
      (point.y - partition_start.y) * (partition_end.x - partition_start.x);
  return (result < 0) ? Child::Left : Child::Right;
}

bool Node::is_node(Child child) const noexcept {
  if (child == Child::Right)
    return std::holds_alternative<Node*>(right_child);
  return std::holds_alternative<Node*>(left_child);
}

bool Node::is_subsector(Child child) const noexcept {
  if (child == Child::Right)
    return std::holds_alternative<Subsector*>(right_child);
  return std::holds_alternative<Subsector*>(left_child);
}

void Node::set_node(Node& node, Child child) noexcept {
  if (child == Child::Right)
    right_child = &node;
  else
    left_child = &node;
}

const Node& Node::get_node(Child child) const {
  if (!is_node(child))
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to get non-node child as a node");

  Node* target_child;
  if (child == Child::Right)
    target_child = std::get<Node*>(right_child);
  else
    target_child = std::get<Node*>(left_child);
  if (!target_child)
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to access uninitialized child");

  return *target_child;
}
Node& Node::get_node(Child child) {
  if (!is_node(child))
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to get non-node child as a node");

  Node* target_child;
  if (child == Child::Right)
    target_child = std::get<Node*>(right_child);
  else
    target_child = std::get<Node*>(left_child);
  if (!target_child)
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to access uninitialized child");

  return *target_child;
}

void Node::set_subsector(Subsector& subsector, Child child) noexcept {
  if (child == Child::Right)
    right_child = &subsector;
  else
    left_child = &subsector;
}

const Subsector& Node::get_subsector(Child child) const {
  if (!is_subsector(child))
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to get non-subsector child as a subsector");

  Subsector* target_child;
  if (child == Child::Right)
    target_child = std::get<Subsector*>(right_child);
  else
    target_child = std::get<Subsector*>(left_child);

  if (!target_child)
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to access uninitialized child");
  return *target_child;
}
Subsector& Node::get_subsector(Child child) {
  if (!is_subsector(child))
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to get non-subsector child as a subsector");

  Subsector* target_child;
  if (child == Child::Right)
    target_child = std::get<Subsector*>(right_child);
  else
    target_child = std::get<Subsector*>(left_child);

  if (!target_child)
    throw BSPException(BSPException::Type::InvalidNodeAccess,
                       "Attempting to access uninitialized child");
  return *target_child;
}
}  // namespace woop