#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <utility>
#include <chrono>
#include <cstdlib>
#include <ctime>

#include <thread>

const int INF = std::numeric_limits<int>::max();

using namespace std;
using namespace std::chrono;

typedef pair<int, int> Edge; // (cost, destination)
typedef vector<vector<Edge>> Graph;

// 產生無向圖
Graph generate_random_graph(int num_nodes, int edges_per_node, int max_weight = 20) {
    Graph graph(num_nodes);
    srand(time(0));

    for (int u = 0; u < num_nodes; ++u) {
        for (int i = 0; i < edges_per_node; ++i) {
            int v = rand() % num_nodes;
            if (v != u) {
                int weight = 1 + rand() % max_weight;
                graph[u].push_back({weight, v});
                graph[v].push_back({weight, u}); // 無向圖
            }
        }
    }
    return graph;
}

vector<int> dijkstra(int source, const vector<vector<Edge>>& graph) {
    int n = graph.size();
    vector<int> dist(n, INF);
    priority_queue<Edge, vector<Edge>, greater<Edge>> pq;

    dist[source] = 0;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;

        for (auto& [cost, v] : graph[u]) {
            if (dist[v] > dist[u] + cost) {
                dist[v] = dist[u] + cost;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

int main() {
    int num_nodes = 1000000;        // 節點數
    int edges_per_node = 5;       // 每個節點平均連接幾條邊

    Graph graph = generate_random_graph(num_nodes, edges_per_node);

    auto start = high_resolution_clock::now();
    vector<int> dist = dijkstra(0, graph);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    // for (int i = 0; i < dist.size(); i++){
    //     cout << i << ':' << dist[i] << ' ';
    // }
    // cout << endl;
    cout << "Elapsed time: " << duration.count() << " ms" << endl;
    return 0;
}