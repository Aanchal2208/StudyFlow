#ifndef ROADMAP_H
#define ROADMAP_H

#include <string>
#include <vector>
#include <unordered_map>

// Edge in the topic graph: (destination, weight = difficulty/time units)
struct Edge {
    std::string to;
    int weight;
};

// Result of a shortest-path query
struct PathResult {
    std::vector<std::string> path;
    int cost;
};

// Build the static curriculum graph (topics + prerequisite weights)
std::unordered_map<std::string, std::vector<Edge>> buildCurriculumGraph();

// Dijkstra's algorithm: shortest weighted path from start to end
PathResult dijkstra(
    const std::unordered_map<std::string, std::vector<Edge>>& graph,
    const std::string& start,
    const std::string& end);

#endif
