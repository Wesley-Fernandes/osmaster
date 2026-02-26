#pragma once

#include "graph.hpp"
#include "haversine.hpp"
#include <iostream>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>
#include <string>
#include <vector>


using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type,
                                               osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

class GraphHandler : public osmium::handler::Handler {
  Graph &graph;

public:
  GraphHandler(Graph &g) : graph(g) {}

  void way(const osmium::Way &way) {
    const char *highway = way.tags()["highway"];
    if (!highway)
      return;

    bool bidirectional = true;
    const char *oneway = way.tags()["oneway"];
    if (oneway &&
        (std::string(oneway) == "yes" || std::string(oneway) == "1")) {
      bidirectional = false;
    }

    for (size_t i = 0; i < way.nodes().size() - 1; ++i) {
      const auto &n1 = way.nodes()[i];
      const auto &n2 = way.nodes()[i + 1];

      if (n1.location() && n2.location()) {
        uint32_t u = graph.get_or_create_node(n1.ref(), n1.lat(), n1.lon());
        uint32_t v = graph.get_or_create_node(n2.ref(), n2.lat(), n2.lon());

        double dist = utils::haversine(n1.lat(), n1.lon(), n2.lat(), n2.lon());
        graph.add_edge(u, v, static_cast<float>(dist), bidirectional);
      }
    }
  }
};

class OsmLoader {
public:
  static void build_graph(const std::string &path, Graph &graph) {
    osmium::io::Reader reader{path};

    index_type index;
    location_handler_type location_handler{index};

    GraphHandler graph_handler{graph};

    osmium::apply(reader, location_handler, graph_handler);
    reader.close();

    std::cout << "Graph built: " << graph.node_count() << " nodes, "
              << graph.edge_count() << " edges." << std::endl;
  }
};
