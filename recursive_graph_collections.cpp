#include "recursive_graph_collections.h"

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <climits>
#include <cstddef>

#ifndef LEVEL_MAX
#define LEVEL_MAX 4
#endif

// anonymous namespace for private helpers and data structures
namespace {
    // We define our data structures using STL containers;
    // each undirected edge is stored twice, once for each endpoint;
    // neighbor is represented as a pair of (node ID, edge level)
    using NodeID = int;
    using EdgeLevel = int;
    using Neighbor = std::pair<NodeID, EdgeLevel>;
    using Neighbors = std::set<Neighbor>;
    using Graph = std::map<NodeID, Neighbors>;
    using Collection = std::map<std::string, Graph>;
    using Collections = std::unordered_map<long, Collection>;

    // our basic structure for the exercise;
    // we use getter instead of global initialization
    Collections& get_collections() {
        static Collections collections;
        return collections;
    }

    Graph* find_graph(long id, char const *name) {
        if (name == nullptr) return nullptr;

        Collections &collections = get_collections();

        auto collectionIt = collections.find(id);
        if (collectionIt == collections.end()) {
            return nullptr;
        }

        auto graphIt = collectionIt->second.find(name);
        if (graphIt == collectionIt->second.end()) {
            return nullptr;
        }

        return &graphIt->second;
    }

    auto find_neighbor(const Neighbors &neighbors, NodeID id) {
        auto neighborIt = neighbors.lower_bound({id, 0});

        if (neighborIt != neighbors.end() && neighborIt->first == id) {
            return neighborIt;
        }

        return neighbors.end();
    }

    bool valid_graph_name(char const *name) {
        if (name == nullptr || *name == '\0') {
            return false;
        }

        for (auto p = name; *p != '\0'; ++p) {
            char c = *p;

            if (!((c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') ||
                  c == '_')) {
                return false;
            }
        }

        return true;
    }
}


namespace cxx {

extern "C" {

long rgc_new_collection() {
    static long nextId = 0;

    if (nextId < 0) {
        return -1;
    }

    long currentId = nextId;

    get_collections().try_emplace(currentId);

    if (nextId == LONG_MAX) {
        nextId = -1;
    } else {
        ++nextId;
    }

    return currentId;
}

void rgc_delete_collection(long id) {
    get_collections().erase(id);
}

bool rgc_new_graph(long id, char const *name, int n) {
    if (n < 0 || !valid_graph_name(name)) {
        return false;
    }

    Collections& collections = get_collections();

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return false;
    }

    auto [graphIt, inserted] =
        collectionIt->second.try_emplace(std::string(name));

    if (!inserted) {
        return false;
    }

    Graph& graph = graphIt->second;

    for (int i = 0; i < n; ++i) {
        graph.try_emplace(i);
    }

    return true;
}

void rgc_delete_graph(long id, char const *name) {
    if (name == nullptr) {
        return;
    }

    auto &collections = get_collections();

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return;
    }

    collectionIt->second.erase(name);
}

int rgc_graph_new_node(long id, char const *name) {
    Graph *graph = find_graph(id, name);

    if (graph == nullptr || graph->size() >= INT_MAX) {
        return -1;
    }

    NodeID mex = 0;

    for (const auto &node : *graph) {
        if (node.first != mex) {
            break;
        }
        ++mex;
    }

    graph->try_emplace(mex);

    return mex;
}

bool rgc_graph_delete_node(long id, char const *name, int x) {
    Graph* graph = find_graph(id, name);
    if (graph == nullptr) {
        return false;
    }

    auto nodeIt = graph->find(x);
    if (nodeIt == graph->end()) {
        return false;
    }

    for (const auto &[neighborId, level] : nodeIt->second) {
        auto neighborIt = graph->find(neighborId);
        neighborIt->second.erase({x, level});
    }

    graph->erase(nodeIt);

    return true;
}

bool rgc_graph_add_edge(long id, char const *name, int x, int y) {
    if (x == y) {
        return false;
    }

    Graph* graph = find_graph(id, name);
    if (graph == nullptr) {
        return false;
    }

    auto nodeX = graph->find(x);
    auto nodeY = graph->find(y);

    if (nodeX == graph->end() || nodeY == graph->end()) {
        return false;
    }

    const Neighbors& neighbors = nodeX->second;

    if (find_neighbor(neighbors, y) != neighbors.end()) {
        return false;
    }

    nodeX->second.emplace(y, 0);
    nodeY->second.emplace(x, 0);

    return true;
}

bool rgc_graph_delete_edge(long id, char const *name, int x, int y) {
    if (x == y) {
        return false;
    }

    Graph* graph = find_graph(id, name);
    if (graph == nullptr) {
        return false;
    }

    auto nodeX = graph->find(x);
    auto nodeY = graph->find(y);

    if (nodeX == graph->end() || nodeY == graph->end()) {
        return false;
    }

    auto neighborXY = find_neighbor(nodeX->second, y);
    if (neighborXY == nodeX->second.end()) {
        return false;
    }

    auto neighborYX = find_neighbor(nodeY->second, x);
    if (neighborYX == nodeY->second.end()) {
        return false;
    }

    nodeX->second.erase(neighborXY);
    nodeY->second.erase(neighborYX);

    return true;
}

char const * rgc_first_graph(long id) {
    const auto &collections = get_collections();

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end() ||
        collectionIt->second.empty()) {
        return nullptr;
    }

    return collectionIt->second.begin()->first.c_str();
}

char const * rgc_next_graph(long id, char const *name) {
    if (name == nullptr) {
        return nullptr;
    }

    const auto &collections = get_collections();

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return nullptr;
    }

    const Collection& collection = collectionIt->second;

    auto graphIt = collection.find(name);
    if (graphIt == collection.end()) {
        return nullptr;
    }

    ++graphIt;

    if (graphIt == collection.end()) {
        return nullptr;
    }

    return graphIt->first.c_str();
}

int rgc_first_node(long id, char const *name) {
    const Graph* graph = find_graph(id, name);

    if (graph == nullptr || graph->empty()) {
        return -1;
    }

    return graph->begin()->first;
}

int rgc_next_node(long id, char const *name, int x) {
    const Graph *graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    auto nodeIt = graph->upper_bound(x);
    if (nodeIt == graph->end()) {
        return -1;
    }

    return nodeIt->first;
}

int rgc_first_edge(long id, char const *name, int x) {
    const Graph *graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    auto nodeIt = graph->find(x);
    if (nodeIt == graph->end()) {
        return -1;
    }

    const Neighbors& neighbors = nodeIt->second;

    if (neighbors.empty()) {
        return -1;
    }

    return neighbors.begin()->first;
}

int rgc_next_edge(long id, char const *name, int x, int y) {
    const Graph *graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    auto nodeIt = graph->find(x);
    if (nodeIt == graph->end()) {
        return -1;
    }

    const Neighbors &neighbors = nodeIt->second;

    // Why does {y, LEVEL_MAX - 1} work? Because upper_bound searches
    // lexicographically through the set, so it will always find
    // a pair where NodeId is strictly greater than y
    auto neighborIt = neighbors.upper_bound({y, LEVEL_MAX - 1});

    if (neighborIt == neighbors.end()) {
        return -1;
    }

    return neighborIt->first;
}

bool rgc_replace_edge(long id, char const *result_name,
    char const *recipient_name, int w_1, int w_2,
    char const *donor_name, int v_1, int v_2) {

    if (!valid_graph_name(result_name) ||
        recipient_name == nullptr || donor_name == nullptr) {
        return false;
    }

    auto &collections = get_collections();
    auto collectionIt = collections.find(id);

    if (collectionIt == collections.end()) {
        return false;
    }

    Collection& collection = collectionIt->second;

    if (collection.contains(result_name)) {
        return false;
    }

    auto recipientIt = collection.find(recipient_name);
    auto donorIt = collection.find(donor_name);

    if (recipientIt == collection.end() ||
        donorIt == collection.end()) {
        return false;
    }

    const Graph &recipient = recipientIt->second;
    const Graph &donor = donorIt->second;

    if (v_1 == v_2 ||
        !donor.contains(v_1) ||
        !donor.contains(v_2)) {
        return false;
    }

    auto nodeIt = recipient.find(w_1);
    if (nodeIt == recipient.end()) {
        return false;
    }

    auto edgeIt = find_neighbor(nodeIt->second, w_2);
    if (edgeIt == nodeIt->second.end()) {
        return false;
    }

    if (donor.size() - 2 >
        static_cast<std::size_t>(INT_MAX) - recipient.size()) {
        return false;
    }

    EdgeLevel donorLevel = 0;
    bool donorHasEdges = false;

    for (const auto &node : donor) {
        for (const auto &neighbor : node.second) {
            donorHasEdges = true;
            donorLevel = std::max(donorLevel, neighbor.second);
        }
    }

    const EdgeLevel replacedLevel = edgeIt->second;

    const ssize_t addedLevel =
        std::max(static_cast<ssize_t>(donorLevel),
                 static_cast<ssize_t>(replacedLevel)) + 1;

    if (donorHasEdges &&
        (addedLevel >= LEVEL_MAX || addedLevel > INT_MAX)) {
        return false;
    }

    Graph result = recipient;

    result.at(w_1).erase({w_2, replacedLevel});
    result.at(w_2).erase({w_1, replacedLevel});

    std::unordered_map<NodeID, NodeID> vertexMap;

    vertexMap.emplace(v_1, w_1);
    vertexMap.emplace(v_2, w_2);

    NodeID nextAvailable = 0;

    for (const auto &[donorId, neighbors] : donor) {
        if (donorId == v_1 || donorId == v_2) {
            continue;
        }

        while (result.contains(nextAvailable)) {
            ++nextAvailable;
        }

        result.try_emplace(nextAvailable);
        vertexMap.emplace(donorId, nextAvailable);
    }

    for (const auto &[donorId, neighbors] : donor) {
        Neighbors& mappedNeighbors =
            result.at(vertexMap.at(donorId));

        for (const auto &[neighborId, oldLevel] : neighbors) {
            mappedNeighbors.emplace(
                vertexMap.at(neighborId),
                static_cast<EdgeLevel>(addedLevel)
            );
        }
    }

    collection.try_emplace(std::string(result_name), std::move(result));

    return true;
}

ssize_t rgc_edge_level(long id, char const *name, int x, int y) {
    const Graph *graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    auto nodeIt = graph->find(x);
    if (nodeIt == graph->end()) {
        return -1;
    }

    const Neighbors& neighbors = nodeIt->second;
    auto neighborIt = find_neighbor(neighbors, y);

    if (neighborIt == neighbors.end()) {
        return -1;
    }

    return neighborIt->second;
}

ssize_t rgc_graph_level(long id, char const *name) {
    const Graph *graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    EdgeLevel maxLevel = 0;
    auto nodeIt = graph->begin();

    while (nodeIt != graph->end()) {
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
    const Graph *graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    return graph->size();
}

ssize_t rgc_count_edges(long id, char const *name) {
    const Graph *graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    auto nodeIt = graph->begin();
    ssize_t degreeSum = 0;
    while (nodeIt != graph->end()) {
        const Neighbors &neighbors = nodeIt->second;
        degreeSum += neighbors.size();
        ++nodeIt;
    }

    return degreeSum / 2;
}

ssize_t rgc_size(long id) {
    Collections &collections = get_collections();
    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    return collectionIt->second.size();
}
}

}