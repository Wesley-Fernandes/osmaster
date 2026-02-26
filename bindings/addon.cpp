#include "astar.hpp"
#include "ch_preprocessor.hpp"
#include "ch_query.hpp"
#include "graph.hpp"
#include "kdtree.hpp"
#include "osm_loader.hpp"
#include <iostream>
#include <memory>
#include <napi.h>

// Global pointer for demo/testing purposes
std::unique_ptr<Graph> global_graph;
std::unique_ptr<spatial::KDTree> global_index;
std::unique_ptr<routing::CHQuery> global_ch_query;

Napi::Value BuildGraph(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string path = info[0].As<Napi::String>();

  try {
    global_graph = std::make_unique<Graph>();
    std::cout << "Building Graph from: " << path << std::endl;
    OsmLoader::build_graph(path, *global_graph);

    std::cout << "Building Spatial Index..." << std::endl;
    global_index = std::make_unique<spatial::KDTree>(global_graph->nodes);

    Napi::Object res = Napi::Object::New(env);
    res.Set("nodes", Napi::Number::New(env, global_graph->node_count()));
    res.Set("edges", Napi::Number::New(env, global_graph->edge_count()));

    return res;
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value FindNearestNode(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!global_index) {
    Napi::Error::New(env, "Graph not loaded").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "Lat and Lon numbers expected")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  double lat = info[0].As<Napi::Number>();
  double lon = info[1].As<Napi::Number>();

  uint32_t idx = global_index->find_nearest(lat, lon);
  const auto &node = global_graph->nodes[idx];

  Napi::Object res = Napi::Object::New(env);
  res.Set("id", Napi::Number::New(env, static_cast<double>(node.osm_id)));
  res.Set("lat", Napi::Number::New(env, node.lat));
  res.Set("lon", Napi::Number::New(env, node.lon));
  res.Set("index", Napi::Number::New(env, idx));

  return res;
}

Napi::Value FindRoute(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!global_graph || !global_index) {
    Napi::Error::New(env, "Graph not loaded").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (info.Length() < 4) {
    Napi::TypeError::New(env, "Expected startLat, startLon, endLat, endLon")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  double sLat = info[0].As<Napi::Number>();
  double sLon = info[1].As<Napi::Number>();
  double eLat = info[2].As<Napi::Number>();
  double eLon = info[3].As<Napi::Number>();

  uint32_t u = global_index->find_nearest(sLat, sLon);
  uint32_t v = global_index->find_nearest(eLat, eLon);

  auto route = routing::AStar::find_path(*global_graph, u, v);

  Napi::Object res = Napi::Object::New(env);
  res.Set("found", Napi::Boolean::New(env, route.found));
  res.Set("distance", Napi::Number::New(env, route.distance));

  if (route.found) {
    Napi::Array pathArr = Napi::Array::New(env, route.node_indices.size());
    for (size_t i = 0; i < route.node_indices.size(); ++i) {
      const auto &node = global_graph->nodes[route.node_indices[i]];
      Napi::Object pNode = Napi::Object::New(env);
      pNode.Set("lat", Napi::Number::New(env, node.lat));
      pNode.Set("lon", Napi::Number::New(env, node.lon));
      pathArr.Set(i, pNode);
    }
    res.Set("path", pathArr);
  }

  return res;
}

Napi::Value RunCHPreprocessing(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!global_graph) {
    Napi::Error::New(env, "Graph not loaded").ThrowAsJavaScriptException();
    return env.Null();
  }

  try {
    std::cout << "Starting CH Preprocessing..." << std::endl;
    routing::CHPreprocessor preprocessor(*global_graph);
    preprocessor.run();
    std::cout << "CH Preprocessing completed." << std::endl;

    std::cout << "Initializing CH Query Engine..." << std::endl;
    global_ch_query = std::make_unique<routing::CHQuery>(*global_graph);

    return Napi::Boolean::New(env, true);
  } catch (const std::exception &e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
}

Napi::Value FindRouteCH(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (!global_ch_query) {
    Napi::Error::New(env, "CH not preprocessed").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (info.Length() < 4) {
    Napi::TypeError::New(env, "Expected startLat, startLon, endLat, endLon")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  double sLat = info[0].As<Napi::Number>();
  double sLon = info[1].As<Napi::Number>();
  double eLat = info[2].As<Napi::Number>();
  double eLon = info[3].As<Napi::Number>();

  uint32_t u = global_index->find_nearest(sLat, sLon);
  uint32_t v = global_index->find_nearest(eLat, eLon);

  auto route = global_ch_query->find_path(u, v);

  Napi::Object res = Napi::Object::New(env);
  res.Set("found", Napi::Boolean::New(env, route.found));
  res.Set("distance", Napi::Number::New(env, route.distance));

  if (route.found) {
    Napi::Array pathArr = Napi::Array::New(env, route.node_indices.size());
    for (size_t i = 0; i < route.node_indices.size(); ++i) {
      const auto &node = global_graph->nodes[route.node_indices[i]];
      Napi::Object pNode = Napi::Object::New(env);
      pNode.Set("lat", Napi::Number::New(env, node.lat));
      pNode.Set("lon", Napi::Number::New(env, node.lon));
      pathArr.Set(i, pNode);
    }
    res.Set("path", pathArr);
  }

  return res;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "buildGraph"),
              Napi::Function::New(env, BuildGraph));
  exports.Set(Napi::String::New(env, "findNearestNode"),
              Napi::Function::New(env, FindNearestNode));
  exports.Set(Napi::String::New(env, "findRoute"),
              Napi::Function::New(env, FindRoute));
  exports.Set(Napi::String::New(env, "runCHPreprocessing"),
              Napi::Function::New(env, RunCHPreprocessing));
  exports.Set(Napi::String::New(env, "findRouteCH"),
              Napi::Function::New(env, FindRouteCH));
  return exports;
}

NODE_API_MODULE(osmaster_native, Init)
