#pragma once

#include "graph.hpp"
#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

namespace routing {

class CHPreprocessor {
public:
  CHPreprocessor(Graph &graph) : graph(graph) {
    node_count = static_cast<uint32_t>(graph.node_count());
    build_backward_adj();
    dists.assign(node_count, std::numeric_limits<double>::infinity());
    visited_nodes.reserve(node_count);
  }

  void run() {
    std::vector<int> priority(node_count);

    // Initial heuristic: Node Degree (Much faster than initial ED calculation)
    // Nodes with lower degree are generally contracted first.
    for (uint32_t i = 0; i < node_count; ++i) {
      priority[i] =
          static_cast<int>(graph.adj[i].size() + backward_adj[i].size());
    }

    // Priority queue for node contraction
    auto cmp = [&](uint32_t a, uint32_t b) {
      return priority[a] > priority[b];
    };
    std::priority_queue<uint32_t, std::vector<uint32_t>, decltype(cmp)> pq(cmp);

    for (uint32_t i = 0; i < node_count; ++i) {
      pq.push(i);
    }

    uint32_t current_rank = 0;
    std::vector<bool> contracted(node_count, false);

    std::cout << "Starting contraction of " << node_count << " nodes..."
              << std::endl;

    while (!pq.empty()) {
      uint32_t v = pq.top();
      pq.pop();

      // Lazy update: calculate true Edge Difference when it reaches top of PQ
      int ed = calculate_edge_difference(v, contracted);
      if (!pq.empty() && ed > priority[pq.top()]) {
        priority[v] = ed;
        pq.push(v);
        continue;
      }

      // Contract node v
      contract_node(v, contracted);
      graph.nodes[v].rank = current_rank++;
      contracted[v] = true;

      if (current_rank % 10000 == 0) {
        std::cout << "Progress: " << current_rank << " / " << node_count
                  << " nodes contracted." << std::endl;
      }
    }
  }

private:
  Graph &graph;
  uint32_t node_count;
  std::vector<std::vector<Edge>> backward_adj;
  std::vector<double> dists;           // Reusable distance vector
  std::vector<uint32_t> visited_nodes; // To clear dists efficiently

  void build_backward_adj() {
    backward_adj.assign(node_count, {});
    for (uint32_t u = 0; u < node_count; ++u) {
      for (const auto &edge : graph.adj[u]) {
        backward_adj[edge.to].push_back(
            {u, edge.weight, edge.is_shortcut, edge.middle_node});
      }
    }
  }

  int calculate_edge_difference(uint32_t v,
                                const std::vector<bool> &contracted = {}) {
    int shortcuts_needed = 0;
    int edges_removed = 0;

    // Outcoming edges from v
    std::vector<Edge> out_edges;
    for (const auto &e : graph.adj[v]) {
      if (contracted.empty() || !contracted[e.to]) {
        out_edges.push_back(e);
        edges_removed++;
      }
    }

    // Incoming edges to v
    std::vector<Edge> in_edges;
    for (const auto &e : backward_adj[v]) {
      if (contracted.empty() || !contracted[e.to]) {
        in_edges.push_back(e);
        edges_removed++;
      }
    }

    for (const auto &in : in_edges) {
      for (const auto &out : out_edges) {
        if (in.to == out.to)
          continue; // Loop

        double uvw_dist = in.weight + out.weight;
        if (witness_search(in.to, out.to, uvw_dist, v, contracted)) {
          shortcuts_needed++;
        }
      }
    }

    return shortcuts_needed - edges_removed;
  }

  bool witness_search(uint32_t u, uint32_t w, double limit, uint32_t v,
                      const std::vector<bool> &contracted) {
    // Dijkstra search from u to w, avoiding v and already contracted nodes
    // Returns true if a path <= limit exists NOT passing through v

    // Clear previous search data efficiently
    for (uint32_t node : visited_nodes) {
      dists[node] = std::numeric_limits<double>::infinity();
    }
    visited_nodes.clear();

    std::priority_queue<std::pair<double, uint32_t>,
                        std::vector<std::pair<double, uint32_t>>,
                        std::greater<>>
        pq;

    pq.push({0.0, u});
    dists[u] = 0.0;
    visited_nodes.push_back(u);

    int settled_nodes = 0;
    const int MAX_SETTLED = 50; // High performance pruning

    while (!pq.empty() && settled_nodes < MAX_SETTLED) {
      auto [d, curr] = pq.top();
      pq.pop();

      if (d > dists[curr])
        continue;
      if (d > limit)
        break;
      if (curr == w)
        return false; // Found a witness!

      settled_nodes++;

      for (const auto &edge : graph.adj[curr]) {
        if (edge.to == v)
          continue;
        if (!contracted.empty() && contracted[edge.to])
          continue;

        double new_dist = d + edge.weight;
        if (new_dist < dists[edge.to]) {
          if (dists[edge.to] == std::numeric_limits<double>::infinity()) {
            visited_nodes.push_back(edge.to);
          }
          dists[edge.to] = new_dist;
          pq.push({new_dist, edge.to});
        }
      }
    }

    return true; // No witness found
  }

  void contract_node(uint32_t v, const std::vector<bool> &contracted) {
    std::vector<Edge> out_edges;
    for (const auto &e : graph.adj[v]) {
      if (!contracted[e.to])
        out_edges.push_back(e);
    }

    std::vector<Edge> in_edges;
    for (const auto &e : backward_adj[v]) {
      if (!contracted[e.to])
        in_edges.push_back(e);
    }

    for (const auto &in : in_edges) {
      for (const auto &out : out_edges) {
        if (in.to == out.to)
          continue;

        double uvw_dist = in.weight + out.weight;
        if (witness_search(in.to, out.to, uvw_dist, v, contracted)) {
          graph.add_shortcut(in.to, out.to, static_cast<float>(uvw_dist), v);
          // Also update backward_adj for future ED calculations
          backward_adj[out.to].push_back(
              {in.to, static_cast<float>(uvw_dist), true, v});
        }
      }
    }
  }
};

} // namespace routing
