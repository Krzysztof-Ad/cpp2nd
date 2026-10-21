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
    // our basic structure for the exercise
    std::unordered_map<long, std::map<std::string, std::map<int, std::set<std::pair<int, int>>>>> mp;
}


namespace cxx {

extern "C" {

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

int rgc_first_node(long id, char const *name) {
    return 0;
}

int rgc_next_node(long id, char const *name, int x) {
    return 0;
}

int rgc_first_edge(long id, char const *name, int x) {
    return 0;
}

int rgc_next_edge(long id, char const *name, int x, int y) {
    return 0;
}

bool rgc_replace_edge(long id, char const *result_name, char const *recipient_name, int w_1, int w_2, char const *donor_name, int v_1, int v_2) {
    return false;
}

ssize_t rgc_edge_level(long id, char const *name, int x, int y) {
    return 0;
}

ssize_t rgc_graph_level(long id, char const *name) {
    return 0;
}

ssize_t rgc_max_level() {
    return LEVEL_MAX - 1;
}

ssize_t rgc_count_nodes(long id, char const *name) {
    return 0;
}

ssize_t rgc_count_edges(long id, char const *name) {
    return 0;
}

ssize_t rgc_size(long id) {
    return 0;
}
}

}