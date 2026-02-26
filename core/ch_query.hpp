#pragma once

#include "graph.hpp"
#include <algorithm>
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>

namespace routing {

struct CHRouteResult {
  bool found = false;
  std::vector<uint32_t> node_indices;
  double distance = std::numeric_limits<double>::infinity();
};

class CHQuery {
public:
  CHQuery(const Graph &graph) : graph(graph) {
    node_count = static_cast<uint32_t>(graph.node_count());
    build_upward_adjs();

    // Initialize vectors for high-performance retrieval
    dist_f.assign(node_count, std::numeric_limits<double>::infinity());
    dist_b.assign(node_count, std::numeric_limits<double>::infinity());
    parent_f.assign(node_count, 0xFFFFFFFF);
    parent_b.assign(node_count, 0xFFFFFFFF);
    timestamp_f.assign(node_count, 0);
    timestamp_b.assign(node_count, 0);
    current_timestamp = 0;
  }

  CHRouteResult find_path(uint32_t start_idx, uint32_t target_idx) {
    if (start_idx >= node_count || target_idx >= node_count) {
      return {false, {}, std::numeric_limits<double>::infinity()};
    }

    if (start_idx == target_idx) {
      return {true, {start_idx}, 0};
    }

    current_timestamp++;
    meeting_node = 0xFFFFFFFF;
    min_dist = std::numeric_limits<double>::infinity();

    std::priority_queue<std::pair<double, uint32_t>,
                        std::vector<std::pair<double, uint32_t>>,
                        std::greater<>>
        pq_f, pq_b;

    pq_f.push({0.0, start_idx});
    dist_f[start_idx] = 0.0;
    timestamp_f[start_idx] = current_timestamp;

    pq_b.push({0.0, target_idx});
    dist_b[target_idx] = 0.0;
    timestamp_b[target_idx] = current_timestamp;

    while (!pq_f.empty() || !pq_b.empty()) {
      // Forward step
      if (!pq_f.empty()) {
        auto [d, u] = pq_f.top();
        pq_f.pop();

        if (d < min_dist) {
          if (timestamp_f[u] == current_timestamp && d <= dist_f[u]) {
            for (const auto &edge : adj_up_forward[u]) {
              double new_dist = d + edge.weight;
              if (timestamp_f[edge.to] != current_timestamp ||
                  new_dist < dist_f[edge.to]) {
                dist_f[edge.to] = new_dist;
                parent_f[edge.to] = u;
                timestamp_f[edge.to] = current_timestamp;
                pq_f.push({new_dist, edge.to});

                if (timestamp_b[edge.to] == current_timestamp) {
                  double total = new_dist + dist_b[edge.to];
                  if (total < min_dist) {
                    min_dist = total;
                    meeting_node = edge.to;
                  }
                }
              }
            }
          }
        }
      }

      // Backward step
      if (!pq_b.empty()) {
        auto [d, u] = pq_b.top();
        pq_b.pop();

        if (d < min_dist) {
          if (timestamp_b[u] == current_timestamp && d <= dist_b[u]) {
            for (const auto &edge : adj_up_backward[u]) {
              double new_dist = d + edge.weight;
              if (timestamp_b[edge.to] != current_timestamp ||
                  new_dist < dist_b[edge.to]) {
                dist_b[edge.to] = new_dist;
                parent_b[edge.to] = u;
                timestamp_b[edge.to] = current_timestamp;
                pq_b.push({new_dist, edge.to});

                if (timestamp_f[edge.to] == current_timestamp) {
                  double total = new_dist + dist_f[edge.to];
                  if (total < min_dist) {
                    min_dist = total;
                    meeting_node = edge.to;
                  }
                }
              }
            }
          }
        }
      }
    }

    if (meeting_node == 0xFFFFFFFF) {
      return {false, {}, std::numeric_limits<double>::infinity()};
    }

    CHRouteResult result;
    result.found = true;
    result.distance = min_dist;

    std::vector<uint32_t> path_packed;
    std::vector<uint32_t> half_f, half_b;

    uint32_t curr = meeting_node;
    while (curr != start_idx) {
      half_f.push_back(curr);
      curr = parent_f[curr];
    }
    half_f.push_back(start_idx);
    std::reverse(half_f.begin(), half_f.end());

    curr = meeting_node;
    while (curr != target_idx) {
      curr = parent_b[curr];
      half_b.push_back(curr);
    }

    path_packed.insert(path_packed.end(), half_f.begin(), half_f.end());
    path_packed.insert(path_packed.end(), half_b.begin(), half_b.end());

    result.node_indices = unpack_path(path_packed);
    return result;
  }

private:
  const Graph &graph;
  uint32_t node_count;
  std::vector<std::vector<Edge>> adj_up_forward;
  std::vector<std::vector<Edge>> adj_up_backward;

  // Reusable search data
  std::vector<double> dist_f, dist_b;
  std::vector<uint32_t> parent_f, parent_b;
  std::vector<uint32_t> timestamp_f, timestamp_b;
  uint32_t current_timestamp;
  uint32_t meeting_node;
  double min_dist;

  void build_upward_adjs() {
    adj_up_forward.assign(node_count, {});
    adj_up_backward.assign(node_count, {});

    for (uint32_t u = 0; u < node_count; ++u) {
      uint32_t rank_u = graph.nodes[u].rank;
      for (const auto &edge : graph.adj[u]) {
        uint32_t rank_v = graph.nodes[edge.to].rank;
        if (rank_v > rank_u) {
          adj_up_forward[u].push_back(edge);
        } else if (rank_v < rank_u) {
          // This edge is an incoming UP edge for v
          adj_up_backward[edge.to].push_back(
              {u, edge.weight, edge.is_shortcut, edge.middle_node});
        }
      }
    }
  }

  std::vector<uint32_t> unpack_path(const std::vector<uint32_t> &packed) {
    if (packed.empty())
      return {};
    std::vector<uint32_t> unpacked;
    unpacked.push_back(packed[0]);

    for (size_t i = 0; i < packed.size() - 1; ++i) {
      unpack_edge(packed[i], packed[i + 1], unpacked);
    }
    return unpacked;
  }

  void unpack_edge(uint32_t u, uint32_t v, std::vector<uint32_t> &path) {
    // Find the edge u->v
    // If it's a shortcut, recursively unpack u->middle and middle->v
    // Else, just add v

    // We need to know if u->v is a shortcut.
    // Let's search in graph.adj[u] for an edge to v with the correct weight
    // (Actually storing shortcut info in a map would be faster)

    const Edge *best_edge = nullptr;
    for (const auto &e : graph.adj[u]) {
      if (e.to == v) {
        if (!best_edge || e.weight < best_edge->weight) {
          best_edge = &e;
        }
      }
    }

    if (best_edge && best_edge->is_shortcut) {
      unpack_edge(u, best_edge->middle_node, path);
      unpack_edge(best_edge->middle_node, v, path);
    } else {
      path.push_back(v);
    }
  }
};

} // namespace routing
