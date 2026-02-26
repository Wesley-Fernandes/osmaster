#pragma once

#include "graph.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>


namespace spatial {

struct KDNode {
  uint32_t index; // Index in the graph.nodes vector
  uint32_t left = std::numeric_limits<uint32_t>::max();
  uint32_t right = std::numeric_limits<uint32_t>::max();
};

class KDTree {
  std::vector<KDNode> tree;
  const std::vector<NodeInfo> &source_nodes;
  uint32_t root = std::numeric_limits<uint32_t>::max();

  struct Point {
    double x, y;
    uint32_t original_index;
  };

  uint32_t build_recursive(std::vector<Point> &points, int depth, int start,
                           int end) {
    if (start >= end)
      return std::numeric_limits<uint32_t>::max();

    int axis = depth % 2;
    int mid = start + (end - start) / 2;

    std::nth_element(points.begin() + start, points.begin() + mid,
                     points.begin() + end,
                     [axis](const Point &a, const Point &b) {
                       return axis == 0 ? a.x < b.x : a.y < b.y;
                     });

    uint32_t node_idx = static_cast<uint32_t>(tree.size());
    tree.push_back({points[mid].original_index});

    tree[node_idx].left = build_recursive(points, depth + 1, start, mid);
    tree[node_idx].right = build_recursive(points, depth + 1, mid + 1, end);

    return node_idx;
  }

  void nearest_recursive(uint32_t node_idx, double qx, double qy, int depth,
                         uint32_t &best_idx, double &best_dist_sq) const {
    if (node_idx == std::numeric_limits<uint32_t>::max())
      return;

    const auto &node = tree[node_idx];
    const auto &p = source_nodes[node.index];

    double dx = qx - p.lat;
    double dy = qy - p.lon;
    double dist_sq = dx * dx + dy * dy;

    if (dist_sq < best_dist_sq) {
      best_dist_sq = dist_sq;
      best_idx = node.index;
    }

    int axis = depth % 2;
    double diff = (axis == 0) ? (qx - p.lat) : (qy - p.lon);

    uint32_t near = diff < 0 ? node.left : node.right;
    uint32_t far = diff < 0 ? node.right : node.left;

    nearest_recursive(near, qx, qy, depth + 1, best_idx, best_dist_sq);

    if (diff * diff < best_dist_sq) {
      nearest_recursive(far, qx, qy, depth + 1, best_idx, best_dist_sq);
    }
  }

public:
  KDTree(const std::vector<NodeInfo> &nodes) : source_nodes(nodes) {
    std::vector<Point> points;
    points.reserve(nodes.size());
    for (uint32_t i = 0; i < nodes.size(); ++i) {
      points.push_back({nodes[i].lat, nodes[i].lon, i});
    }
    tree.reserve(nodes.size());
    root = build_recursive(points, 0, 0, static_cast<int>(points.size()));
  }

  uint32_t find_nearest(double lat, double lon) const {
    uint32_t best_idx = std::numeric_limits<uint32_t>::max();
    double best_dist_sq = std::numeric_limits<double>::max();
    nearest_recursive(root, lat, lon, 0, best_idx, best_dist_sq);
    return best_idx;
  }
};

} // namespace spatial
