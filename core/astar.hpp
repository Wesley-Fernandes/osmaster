#pragma once

#include "graph.hpp"
#include "haversine.hpp"
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>


namespace routing {

struct RouteResult {
  bool found = false;
  std::vector<uint32_t> node_indices;
  double distance = 0; // Total distance in meters
};

struct AStarNode {
  uint32_t index;
  double g_score; // Distance from start
  double f_score; // g_score + heuristic (haversine to target)

  bool operator>(const AStarNode &other) const {
    return f_score > other.f_score;
  }
};

class AStar {
public:
  static RouteResult find_path(const Graph &graph, uint32_t start_idx,
                               uint32_t target_idx) {
    if (start_idx >= graph.node_count() || target_idx >= graph.node_count()) {
      return {false, {}, 0};
    }

    const auto &target_node = graph.nodes[target_idx];

    std::priority_queue<AStarNode, std::vector<AStarNode>,
                        std::greater<AStarNode>>
        pq;
    std::unordered_map<uint32_t, double> g_scores;
    std::unordered_map<uint32_t, uint32_t> came_from;

    g_scores[start_idx] = 0;
    pq.push({start_idx, 0,
             utils::haversine(graph.nodes[start_idx].lat,
                              graph.nodes[start_idx].lon, target_node.lat,
                              target_node.lon)});

    while (!pq.empty()) {
      AStarNode current = pq.top();
      pq.pop();

      if (current.index == target_idx) {
        // Path found, reconstruct it
        RouteResult result;
        result.found = true;
        result.distance = g_scores[target_idx];

        uint32_t curr_reconstruct = target_idx;
        while (curr_reconstruct != start_idx) {
          result.node_indices.push_back(curr_reconstruct);
          curr_reconstruct = came_from[curr_reconstruct];
        }
        result.node_indices.push_back(start_idx);
        std::reverse(result.node_indices.begin(), result.node_indices.end());

        return result;
      }

      if (current.g_score > g_scores[current.index])
        continue;

      for (const auto &edge : graph.adj[current.index]) {
        double tentative_g = current.g_score + edge.weight;

        if (g_scores.find(edge.to) == g_scores.end() ||
            tentative_g < g_scores[edge.to]) {
          came_from[edge.to] = current.index;
          g_scores[edge.to] = tentative_g;

          double h = utils::haversine(graph.nodes[edge.to].lat,
                                      graph.nodes[edge.to].lon, target_node.lat,
                                      target_node.lon);
          pq.push({edge.to, tentative_g, tentative_g + h});
        }
      }
    }

    return {false, {}, 0};
  }
};

} // namespace routing
