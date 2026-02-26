#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

struct Edge {
  uint32_t to;
  float weight; // Distance in meters
  bool is_shortcut = false;
  uint32_t middle_node = 0xFFFFFFFF; // For shortcut reconstruction
};

struct NodeInfo {
  int64_t osm_id;
  double lat;
  double lon;
  uint32_t rank = 0xFFFFFFFF; // CH rank, default max
};

class Graph {
public:
  std::vector<NodeInfo> nodes;
  std::vector<std::vector<Edge>> adj;
  std::unordered_map<int64_t, uint32_t> osm_to_internal;

  uint32_t get_or_create_node(int64_t osm_id, double lat = 0, double lon = 0) {
    auto it = osm_to_internal.find(osm_id);
    if (it != osm_to_internal.end()) {
      if (lat != 0 && lon != 0) {
        nodes[it->second].lat = lat;
        nodes[it->second].lon = lon;
      }
      return it->second;
    }

    uint32_t internal_id = static_cast<uint32_t>(nodes.size());
    nodes.push_back({osm_id, lat, lon});
    adj.push_back({});
    osm_to_internal[osm_id] = internal_id;
    return internal_id;
  }

  void add_edge(uint32_t from, uint32_t to, float weight,
                bool bidirectional = false) {
    adj[from].push_back({to, weight});
    if (bidirectional) {
      adj[to].push_back({from, weight});
    }
  }

  void add_shortcut(uint32_t from, uint32_t to, float weight, uint32_t middle) {
    adj[from].push_back({to, weight, true, middle});
  }

  void clear() {
    nodes.clear();
    adj.clear();
    osm_to_internal.clear();
  }

  void reset_ranks() {
    for (auto &node : nodes) {
      node.rank = 0xFFFFFFFF;
    }
  }

  size_t node_count() const { return nodes.size(); }
  size_t edge_count() const {
    size_t count = 0;
    for (const auto &list : adj)
      count += list.size();
    return count;
  }
};
