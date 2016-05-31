#include <algorithm>
#include "dort/box.hpp"
#include "dort/geometry.hpp"
#include "dort/kd_tree.hpp"
#include "dort/photon_map.hpp"

namespace dort {
  template<class Traits>
  KdTree<Traits>::KdTree(std::vector<Element> elems):
    elements(std::move(elems))
  {
    if(!this->elements.empty()) {
      this->nodes.reserve(this->elements.size());
      this->build_node(0, this->elements.size());
    }
  }

  template<class Traits>
  void KdTree<Traits>::lookup(const Point& p, float radius_square,
      std::function<float(const Element&, float, float)> callback) const
  {
    if(this->elements.empty()) {
      return;
    }

    std::array<uint32_t, 30> todo_stack;
    uint32_t todo_top = 0;
    uint32_t todo_node_idx = 0;

    for(;;) {
      uint32_t node_idx = todo_node_idx;
      const Node& node = this->nodes.at(node_idx);
      const Element& elem = this->elements.at(node_idx);
      uint8_t axis = node.split_axis();

      float point_dist_square = length_squared(p - Traits::element_point(elem));
      if(point_dist_square <= radius_square) {
        radius_square = callback(elem, point_dist_square, radius_square);
      }

      if(axis != 3) {
        uint32_t left_idx = node_idx + 1;
        uint32_t right_idx = node.right_child();
        float split_dist_square = square(p.v[axis] - node.split_pos);

        if(p.v[axis] < node.split_pos) {
          if(right_idx != Node::NO_RIGHT && split_dist_square < radius_square) {
            todo_stack.at(todo_top++) = right_idx;
          }
          todo_node_idx = left_idx;
          continue;
        } else {
          if(split_dist_square < radius_square) {
            todo_stack.at(todo_top++) = left_idx;
          }
          if(right_idx != Node::NO_RIGHT) {
            todo_node_idx = right_idx;
            continue;
          }
        }
      }

      if(todo_top == 0) {
        break;
      }
      todo_node_idx = todo_stack.at(--todo_top);
    }
  }

  template<class Traits>
  void KdTree<Traits>::build_node(uint32_t begin, uint32_t end) {
    assert(begin < end); assert(end <= this->elements.size());
    assert(this->nodes.size() == begin);

    if(begin + 1 == end) {
      this->nodes.push_back(Node(SIGNALING_NAN, 3, Node::NO_RIGHT));
      return;
    }

    Box bounds;
    for(uint32_t i = begin; i < end; ++i) {
      bounds = union_box(bounds, Traits::element_point(this->elements.at(i)));
    }

    uint8_t split_axis = bounds.max_axis();
    uint32_t mid = begin + (end - begin + 1) / 2 + 1;
    assert(begin + 1 < mid); assert(mid <= end);
    std::nth_element(this->elements.begin() + begin,
        this->elements.begin() + mid, this->elements.begin() + end,
        [=](const Element& elem1, const Element& elem2) {
          return Traits::element_point(elem1).v[split_axis] <
            Traits::element_point(elem2).v[split_axis];
        });
    std::swap(this->elements.at(mid), this->elements.at(begin));
    float split_pos = Traits::element_point(this->elements.at(begin)).v[split_axis];

    this->nodes.push_back(Node(split_pos, split_axis,
          mid < end ? mid : Node::NO_RIGHT));
    this->build_node(begin + 1, mid);
    if(mid < end) {
      this->build_node(mid, end);
    }
  }

  template class KdTree<PhotonMap::KdTraits>;
}
