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

    // our basic structure for the exercise;
    // we use getter instead of global initialization
    Collections& get_collections() {
        static Collections collections;
        return collections;
    }

    Graph* find_graph(long id, char const *name) {
        if (name == nullptr) return nullptr;

        Collections& collections = get_collections();

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

    auto find_neighbor(const Neighbors& neighbors, NodeID id) {
        auto neighborIt = neighbors.lower_bound({id, 0});

        if (neighborIt != neighbors.end() && neighborIt->first == id) {
            return neighborIt;
        }

        return neighbors.end();
    }

    bool valid_graph_name(char const* name) {
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

bool rgc_new_graph(long id, char const* name, int n) {
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

    auto& collections = get_collections();

    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return;
    }

    collectionIt->second.erase(name);
}

int rgc_graph_new_node(long id, char const *name) {
    Graph* graph = find_graph(id, name);

    if (graph == nullptr || graph->size() >= INT_MAX) {
        return -1;
    }

    NodeID mex = 0;

    for (const auto& node : *graph) {
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

    for (const auto& [neighborId, level] : nodeIt->second) {
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
    const auto& collections = get_collections();

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

    const auto& collections = get_collections();

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
    const Graph* graph = find_graph(id, name);

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
    const Graph* graph = find_graph(id, name);

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
    const Graph* graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    auto nodeIt = graph->find(x);
    if (nodeIt == graph->end()) {
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
        // TODO: Check if the graph is always undirected

        const Graph* donor_graph = find_graph(id, donor_name); 

        if (donor_graph == nullptr) {
            return false;
        }

        Graph* receiving_graph = find_graph(id, recipient_name);

        if (receiving_graph == nullptr) {
            return false;
        }

        // There already exists a graph with the result name,
        // therefore it is prohibited to create a new one

        if (find_graph(id, result_name) != nullptr) {
            return false;
        }

        // Check if the vertices w_1 and w_2 are in the recipient graph
        // and if there exists an edge between them

        // Check if vertices v_1 and v_2 exist in the donor graph

        auto w_1_it = receiving_graph->find(w_1);
        auto w_2_it = receiving_graph->find(w_2);

        if (w_1_it == receiving_graph->end() || w_2_it == receiving_graph->end()) {
            return false;
        }

        // TODO: Determine if both vertices need to check if there is an edge

        auto edge_it = w_1_it->second.lower_bound({w_2, 0});

        if (edge_it == w_1_it->second.end() || edge_it->first != w_2) {
            return false;
        }

        auto v_1_it = donor_graph->find(v_1);
        auto v_2_it = donor_graph->find(v_2);

        if (v_1_it == donor_graph->end() || v_2_it == donor_graph->end()) {
            return false;
        }

        // TODO: Use a linter or other tool to correct the formatting

        ssize_t replaced_edge_level_r = rgc_edge_level(id, recipient_name, w_1, w_2);
        ssize_t replaced_edge_level_d = rgc_edge_level(id, donor_name, v_1, v_2);
 
        // If the replaced edge does not exist, then no levels have increased
        // Therefore it is impossible to exceed the maximum limit

        if (replaced_edge_level_d != -1 && replaced_edge_level_r >= rgc_max_level() - 1) {
            return false;
        }

        // Check if the total number of vertices does not exceed INT_MAX
        // TODO: Verify very thoroughly that no integer overflows may occur

        if ((receiving_graph->size() - 2) >= (INT_MAX - donor_graph->size())) {
            return false;
        }

        // Begin the procedure of replacing the edge

        // Create the new result graph that is a copy of the receiving one

        Collections& collections = get_collections();
        auto collection_it = collections.find(id);
        
        collection_it->second[result_name] = (*receiving_graph);
        Graph* result_graph = &collection_it->second[result_name];

        // Remove the edge from the result graph
        // It is certain that the edge exists so
        // there is no need to check edge cases

        auto edge_end_1 = result_graph->find(w_2)->second.lower_bound({w_1, 0});
        auto edge_end_2 = result_graph->find(w_1)->second.lower_bound({w_2, 0});

        result_graph->find(w_2)->second.erase(edge_end_1);
        result_graph->find(w_1)->second.erase(edge_end_2);

        // Insert a new vertex donor_graph->size() - 2 times 
        // Keep an unordered map that will transform old IDs into new ones
        // Add edges one by one

        // Maybe consider adding a more descriptive name
        std::unordered_map<int, int> indices;

        indices[v_1] = w_1;
        indices[v_2] = w_2;

        // Determine if the types are in accordance with the requirements
        // Consider using a more modern approach for iterating

        int donor_graph_size = donor_graph->size();
        
        for (auto donor_graph_it = donor_graph->begin(); 
            donor_graph_it != donor_graph->end(); donor_graph_it++) {

            // The first two vertices have already been associated
            // with vertices, otherwise we do this in order

            if (donor_graph_it->first == v_1 || donor_graph_it->first == v_2) {
                continue;
            }

            int new_node_number = rgc_graph_new_node(id, result_name);
            indices[donor_graph_it->first] = new_node_number; 
        }


        for (auto donor_graph_it = donor_graph->begin(); 
            donor_graph_it != donor_graph->end(); donor_graph_it++) {

            int origin = donor_graph_it->first;
            

            for (auto head_it = donor_graph_it->second.begin(); 
                head_it != donor_graph_it->second.end(); head_it++) {
                    auto origin_result = result_graph->find(indices[origin]);
                    int current_level = head_it->second + 1;

                    if ((indices[head_it->first] == w_1 && indices[origin] == w_2) || 
                    (indices[head_it->first] == w_2 && indices[origin] == w_1)) {
                        current_level = std::max(current_level, (int) replaced_edge_level_r);
                    }


                    origin_result->second.insert({indices[head_it->first], current_level});
                }
        }

        return true;
    }
// ==============================

ssize_t rgc_edge_level(long id, char const *name, int x, int y) {
    const Graph* graph = find_graph(id, name);

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
    const Graph* graph = find_graph(id, name);

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
    const Graph* graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    return graph->size();
}

ssize_t rgc_count_edges(long id, char const *name) {
    const Graph* graph = find_graph(id, name);

    if (graph == nullptr) {
        return -1;
    }

    auto nodeIt = graph->begin();
    ssize_t degreeSum = 0;
    while (nodeIt != graph->end()) {
        const Neighbors& neighbors = nodeIt->second;
        degreeSum += neighbors.size();
        ++nodeIt;
    }

    return degreeSum / 2;
}

ssize_t rgc_size(long id) {
    Collections& collections = get_collections();
    auto collectionIt = collections.find(id);
    if (collectionIt == collections.end()) {
        return -1;
    }

    return collectionIt->second.size();
}
}

}
