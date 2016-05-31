#pragma once
#include <vector>
#include "dort/dort.hpp"

namespace dort {
  template<class Traits>
  class KdTree final {
  public:
    using Element = typename Traits::Element;
  private:
    struct Node {
      float split_pos;
      uint32_t axis_child;

      Node() = default;
      Node(float split_pos, uint8_t split_axis, uint32_t right_child) {
        this->split_pos = split_pos;
        this->axis_child = uint32_t(split_axis) + (right_child << 2);
      }

      uint8_t split_axis() const {
        return uint8_t(this->axis_child & 3);
      }
      uint32_t right_child() const {
        return this->axis_child >> 2;
      }

      static constexpr uint32_t NO_RIGHT = -1u >> 2;
    };

    std::vector<Node> nodes;
    std::vector<Element> elements;
  public:
    KdTree() = default;
    KdTree(std::vector<Element> elems);

    void lookup(const Point& p, float radius_square,
        std::function<float(const Element& e, float d2, float r2)> callback) const;
  private:
    void build_node(uint32_t begin, uint32_t end);
  };
}
