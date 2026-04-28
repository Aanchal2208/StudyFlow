#include "roadmap.h"
#include <queue>
#include <unordered_set>
#include <limits>

using namespace std;

/* Builds the canonical CS-curriculum graph used by the frontend.
   Weights represent relative difficulty/time-to-master between
   consecutive topics. Same data is mirrored client-side. */
unordered_map<string, vector<Edge>> buildCurriculumGraph() {
    unordered_map<string, vector<Edge>> g;
    g["Basics of Programming"] = {{"Data Types",1},{"Control Flow",2}};
    g["Data Types"]            = {{"Control Flow",1},{"Functions",2}};
    g["Control Flow"]          = {{"Functions",1},{"Arrays",3}};
    g["Functions"]             = {{"Arrays",2},{"Recursion",3}};
    g["Arrays"]                = {{"Pointers",2},{"Sorting Algorithms",3},{"Searching Algorithms",2}};
    g["Pointers"]              = {{"Linked Lists",2},{"OOP",3}};
    g["Recursion"]             = {{"Linked Lists",2},{"Trees",3},{"Dynamic Programming",4}};
    g["OOP"]                   = {{"Linked Lists",1},{"Advanced DSA",6}};
    g["Linked Lists"]          = {{"Stacks & Queues",2},{"Trees",3}};
    g["Stacks & Queues"]       = {{"Trees",2},{"Graphs",3}};
    g["Trees"]                 = {{"Graphs",2},{"Advanced DSA",4}};
    g["Graphs"]                = {{"Graph Algorithms",2},{"Advanced DSA",3}};
    g["Sorting Algorithms"]    = {{"Searching Algorithms",1},{"Greedy Algorithms",3}};
    g["Searching Algorithms"]  = {{"Dynamic Programming",4}};
    g["Dynamic Programming"]   = {{"Advanced DSA",3}};
    g["Greedy Algorithms"]     = {{"Graph Algorithms",2},{"Advanced DSA",3}};
    g["Graph Algorithms"]      = {{"Advanced DSA",2}};
    g["Advanced DSA"]          = {};
    return g;
}

/* Standard Dijkstra implementation using a min-heap.
   Returns the path (in order) and total weighted cost. */
PathResult dijkstra(
    const unordered_map<string, vector<Edge>>& graph,
    const string& start, const string& end)
{
    unordered_map<string,int> dist;
    unordered_map<string,string> prev;
    for (auto& kv : graph) dist[kv.first] = numeric_limits<int>::max();
    dist[start] = 0;

    // Min-heap of (distance, node)
    using P = pair<int,string>;
    priority_queue<P, vector<P>, greater<P>> pq;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (u == end) break;
        if (d > dist[u]) continue;
        auto it = graph.find(u);
        if (it == graph.end()) continue;
        for (const auto& e : it->second) {
            int alt = d + e.weight;
            if (alt < dist[e.to]) {
                dist[e.to] = alt;
                prev[e.to] = u;
                pq.push({alt, e.to});
            }
        }
    }

    PathResult res;
    res.cost = dist.count(end) ? dist[end] : -1;
    if (res.cost == numeric_limits<int>::max()) { res.cost = -1; return res; }

    // Reconstruct path by walking predecessor links
    string cur = end;
    while (cur != "") {
        res.path.insert(res.path.begin(), cur);
        if (cur == start) break;
        if (!prev.count(cur)) { res.path.clear(); res.cost = -1; break; }
        cur = prev[cur];
    }
    return res;
}
