#include "recursive_graph_collections.h"

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

#ifndef LEVEL_MAX
#define LEVEL_MAX 4
#endif

// anonymous namespace for private helpers and data structures
namespace {
    using NodeID = int;
    using EdgeLevel = int;
    using Neighbor = std::pair<NodeID, EdgeLevel>;
    using Neighbors = std::set<Neighbor>;
    using Graph = std::map<NodeID, Neighbors>;
    using Collection = std::map<std::string, Graph>;
    using Collections = std::unordered_map<long, Collection>;

    // our basic structure for the exercise
    Collections collections;

    // original structure for reference (without 'using' statements)
    //std::unordered_map<long, std::map<std::string, std::map<int, std::set<std::pair<int, int>>>>> mp;
}


namespace cxx {

extern "C" {

//    Antek
//      |
//      |
//     \|/
//      `

long rgc_new_collection() {
    return 0;
}

void rgc_delete_collection(long id) {

}

bool rgc_new_graph(long id, char const *name, int n) {
    return false;
}

void rgc_delete_graph(long id, char const *name) {

}

int rgc_graph_new_node(long id, char const *name) {
    return 0;
}

bool rgc_graph_delete_node(long id, char const *name, int x) {
    return false;
}

bool rgc_graph_add_edge(long id, char const *name, int x, int y) {
    return false;
}

bool rgc_graph_delete_edge(long id, char const *name, int x, int y) {
    return false;
}

char const * rgc_first_graph(long id) {
    return "";
}

char const * rgc_next_graph(long id, char const *name) {
    return "";
}

//    Antek
// =============
//   Krzysztof
//      |
//      |
//     \|/
//      `

int rgc_first_node(long id, char const *name) {
    if (name == nullptr) return -1;

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    auto graphIt = collectionIt->second.find(name);
    if (graphIt == collectionIt->second.end()) {
        return -1;
    }

    const Graph& graph = graphIt->second;
    if (graph.empty()) {
        return -1;
    }

    return graph.begin()->first;
}

int rgc_next_node(long id, char const *name, int x) {
    if (name == nullptr) return -1;

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    auto graphIt = collectionIt->second.find(name);
    if (graphIt == collectionIt->second.end()) {
        return -1;
    }

    const Graph& graph = graphIt->second;

    auto nodeIt = graph.upper_bound(x);
    if (nodeIt == graph.end()) {
        return -1;
    }

    return nodeIt->first;
}

int rgc_first_edge(long id, char const *name, int x) {
    if (name == nullptr) return -1;

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    auto graphIt = collectionIt->second.find(name);
    if (graphIt == collectionIt->second.end()) {
        return -1;
    }

    const Graph& graph = graphIt->second;

    auto nodeIt = graph.find(x);
    if (nodeIt == graph.end()) {
        return -1;
    }

    const Neighbors& neighbors = nodeIt->second;

    if (neighbors.empty()) {
        return -1;
    }

    return neighbors.begin()->first;
}

int rgc_next_edge(long id, char const *name, int x, int y) {
    if (name == nullptr) return -1;

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    auto graphIt = collectionIt->second.find(name);
    if (graphIt == collectionIt->second.end()) {
        return -1;
    }

    const Graph& graph = graphIt->second;

    auto nodeIt = graph.find(x);
    if (nodeIt == graph.end()) {
        return -1;
    }

    const Neighbors& neighbors = nodeIt->second;

    // Why does {y, LEVEL_MAX - 1} work? Because upper_bound searches
    // lexicographically through the set, so it will always find
    // a pair where NodeId is strictly greater than y
    auto neighborIt = neighbors.upper_bound({y, LEVEL_MAX - 1});

    if (neighborIt == neighbors.end()) {
        return -1;
    }

    return neighborIt->first;
}

// ==============================
// Collaboration on this function
bool rgc_replace_edge(long id, char const *result_name, char const *recipient_name, int w_1, int w_2, char const *donor_name, int v_1, int v_2) {
    return false;
}
// ==============================

ssize_t rgc_edge_level(long id, char const *name, int x, int y) {
    if (name == nullptr) return -1;

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    auto graphIt = collectionIt->second.find(name);
    if (graphIt == collectionIt->second.end()) {
        return -1;
    }

    const Graph& graph = graphIt->second;

    auto nodeIt = graph.find(x);
    if (nodeIt == graph.end()) {
        return -1;
    }

    const Neighbors& neighbors = nodeIt->second;
    auto neighborIt = neighbors.lower_bound({y, 0});
    if (neighborIt == neighbors.end() || neighborIt->first != y) {
        return -1;
    }

    return neighborIt->second;
}

ssize_t rgc_graph_level(long id, char const *name) {
    return 0;
}

ssize_t rgc_max_level() {
    return LEVEL_MAX - 1;
}

ssize_t rgc_count_nodes(long id, char const *name) {
    if (name == nullptr) return -1;

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    auto graphIt = collectionIt->second.find(name);
    if (graphIt == collectionIt->second.end()) {
        return -1;
    }

    return graphIt->second.size();
}

ssize_t rgc_count_edges(long id, char const *name) {
    if (name == nullptr) return -1;

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    auto graphIt = collectionIt->second.find(name);
    if (graphIt == collectionIt->second.end()) {
        return -1;
    }

    const Graph& graph = graphIt->second;

    auto nodeIt = graph.begin();
    ssize_t degreeSum = 0;
    while (nodeIt != graph.end()) {
        const Neighbors& neighbors = nodeIt->second;
        degreeSum += neighbors.size();
        ++nodeIt;
    }

    return degreeSum / 2;
}

ssize_t rgc_size(long id) {
    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    return collectionIt->second.size();
}
//   Krzysztof
// =============
}

}