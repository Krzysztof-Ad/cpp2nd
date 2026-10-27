#include "recursive_graph_collections.h"

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <climits>

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
    // Check if a static variable is sufficient
    // and can replace a global one

    static long next_id = 0;
    
    if (next_id < 0 ) {
        return -1;
    }

    long current_id = next_id;
    collections[current_id] = Collection();

    if (next_id == LONG_MAX) {
        next_id = -1;
    }
    else {
        next_id++;
    }

    return current_id;
}

void rgc_delete_collection(long id) {
    collections.erase(id);
}

bool rgc_new_graph(long id, char const *name, int n) {
    if (n < 0 || name == nullptr) {
        return false;
    }

    auto col_it = collections.find(id);

    if (col_it == collections.end()) {
        return false;
    }

    std::string graph_name(name);

    // We cannot create a new graph when there is already one
    // that has been given the same name in selected the collection

    if (col_it->second.find(graph_name) != col_it->second.end()) {
        return false;
    }

    Graph new_graph;

    for (int i = 0; i < n; i++) {
        new_graph[i] = Neighbors();
    }

    // There is no need to create a deep copy

    col_it->second[graph_name] = std::move(new_graph);

    return true; 
}

void rgc_delete_graph(long id, char const *name) {
    auto col_it = collections.find(id);

    if (col_it == collections.end() || name == nullptr) {
        return;
    }

    col_it->second.erase(name);
}

int rgc_graph_new_node(long id, char const *name) {
    if (name == nullptr) {
        return -1;
    }

    auto col_it = collections.find(id);

    if (col_it == collections.end()) {
        return -1;
    }

    auto graph_it = col_it->second.find(name);

    // Return -1 if no graph has this name
    
    if (graph_it == col_it->second.end()) {
        return -1;
    }

    Graph& graph = graph_it->second;

    // It is forbidden to exceed the maximum size

    if (graph.size() >= INT_MAX) {
        return -1;
    }

    // TODO: It is important to verify if this approach for computing
    // the minimum excluded element is sufficient. There are other ways
    // that may yield a better time complexity, although at the expense
    // of more memory usage.

    int mex = 0;

    for (auto const& [node_id, neighbors] : graph) {
        if (node_id == mex) {
            mex++;
        }
        else {
            break; 
        }
    }

    graph[mex] = Neighbors();
    
    return mex;
}

bool rgc_graph_delete_node(long id, char const *name, int x) {
    // Check edge cases and remove the vertex
    // otherwise report an error   
    
    if (name == nullptr) {
        return false;
    }

    auto col_it = collections.find(id);

    if (col_it == collections.end()) {
        return false;
    }

    auto graph_it = col_it->second.find(name);

    if (graph_it == col_it->second.end()) {
        return false;
    }

    if (graph_it->second.find(x) == graph_it->second.end()) {
        return false;
    }

    // TODO: erase all the edges and only then the vertex
    // The edges are bidirectional so neighbors must be accounted for


    return true;
}

bool rgc_graph_add_edge(long id, char const *name, int x, int y) {
    // The graph must have a name
    
    if (name == nullptr) {
        return false;
    }

    auto col_it = collections.find(id);

    // There must be a collection with the given id

    if (col_it == collections.end()) {
        return false;
    }

    auto graph_it = col_it->second.find(name);

    // There must be a graph named this way for the function
    // to be able to modify it

    if (graph_it == col_it->second.end()) {
        return false;
    }

    Graph& graph = graph_it->second;
    auto it_x = graph.find(x);
    auto it_y = graph.find(y);

    // Both vertices must be in the graph

    if (it_x == graph.end() || it_y == graph.end()) {
        return false;
    }

    // Checking if there already exists an edge

    auto neighbor_it = it_x->second.lower_bound({y, 0});

    if (neighbor_it != it_x->second.end() && neighbor_it->first == y) {
        return false;
    }

    it_x->second.insert({y, 0});
    it_y->second.insert({x, 0});

    return true;
}

bool rgc_graph_delete_edge(long id, char const *name, int x, int y) {
    // The graph must have a name
    
    if (name == nullptr) {
        return false;
    }

    auto col_it = collections.find(id);

    // There must be a collection with the given id

    if (col_it == collections.end()) {
        return false;
    }

    auto graph_it = col_it->second.find(name);

    // There must be a graph named this way for the function
    // to be able to modify it

    if (graph_it == col_it->second.end()) {
        return false;
    }

    Graph& graph = graph_it->second;
    auto it_x = graph.find(x);
    auto it_y = graph.find(y);

    // Both vertices must be in the graph

    if (it_x == graph.end() || it_y == graph.end()) {
        return false;
    }

    // Checking if there already exists an edge
    // IMPORTANT: If something goes south and the edges have not been
    // added symetrically, there may be an error regarding erasing

    auto neighbor_it = it_x->second.lower_bound({y, 0});

    if (neighbor_it == it_x->second.end()) {
        return false;
    }

    if (neighbor_it->first != y) {
        return false;
    }

    it_x->second.erase(neighbor_it);
    it_y->second.erase(it_y->second.lower_bound({x, 0}));

    return true;
}

char const * rgc_first_graph(long id) {
    auto col_it = collections.find(id);

    // There is no first graph if there is no such collection
    // or the collection with the given id is empty

    if (col_it == collections.end() || col_it->second.empty()) {
        return nullptr;
    }

    return col_it->second.begin()->first.c_str();
}

char const * rgc_next_graph(long id, char const *name) {
    if (name == nullptr) {
        return nullptr;
    }

    auto col_it = collections.find(id);

    if (col_it == collections.end() || col_it->second.empty()) {
        return nullptr;
    }

    auto graph_it = col_it->second.find(name);

    // If there is no such name then there is no successor of this name

    if (graph_it == col_it->second.end()) {
        return nullptr;
    }

    ++graph_it;

    // If the successor does not exist then it cannot be returned

    if (graph_it == col_it->second.end()) {
        return nullptr;
    }

    return graph_it->first.c_str();
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

    EdgeLevel maxLevel = 0;
    auto nodeIt = graph.begin();

    while (nodeIt != graph.end()) {
        const Neighbors& neighbors = nodeIt->second;
        auto neighborIt = neighbors.begin();
        while (neighborIt != neighbors.end()) {
            maxLevel = std::max(maxLevel, neighborIt->second);
            ++neighborIt;
        }
        ++nodeIt;
    }

    return maxLevel;
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