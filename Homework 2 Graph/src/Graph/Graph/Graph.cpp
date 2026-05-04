#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <stack>
#include <string>
#include <iomanip>

using namespace std;

// 1. 定義邊結構
struct Edge {
    int u, v, weight;
    // 重載運算子以支援 priority_queue (小頂堆)
    bool operator>(const Edge& o) const { return weight > o.weight; }
    // 重載運算子以支援 sort (由小到大)
    bool operator<(const Edge& o) const { return weight < o.weight; }
};

// 2. 併查集實作
struct DisjointSet {
    vector<int> parent;
    DisjointSet(int n) {
        parent.resize(n);
        for (int i = 0; i < n; i++) parent[i] = i; // 每個點的老大初始為自己
    }

    // 尋找根節點 (無路徑壓縮)
    int find(int i, long long& ops) {
        ops++;
        if (parent[i] == i) return i;
        return find(parent[i], ops);
    }
};

class Graph {
private:
    int n, e, num;
    int* dfn, * low;
    bool* visited;
    vector<pair<int, int>>* adj;
    vector<Edge> all_edges;
    stack<Edge> s_edges;

    // 效能驗證計數器
    long long v_ops_dfs, v_ops_bfs, v_ops_kruskal, v_ops_prim, v_ops_bicon;
    long long v_space_bfs, v_space_kruskal, v_space_prim, v_space_bicon;

public:
    Graph(int nodes) : n(nodes), e(0), num(0) {
        adj = new vector<pair<int, int>>[n];
        dfn = new int[n]; low = new int[n]; visited = new bool[n];
        v_ops_dfs = v_ops_bfs = v_ops_kruskal = v_ops_prim = v_ops_bicon = 0;
        v_space_bfs = v_space_kruskal = v_space_prim = v_space_bicon = 0;
    }

    ~Graph() { delete[] adj; delete[] dfn; delete[] low; delete[] visited; }

    void InsertEdge(int u, int v, int w = 1) {
        adj[u].push_back({ v, w });
        adj[v].push_back({ u, w });
        all_edges.push_back({ u, v, w });
        e++;
    }

    void DFS(int startNode) {
        fill(visited, visited + n, false);
        stack<int> s;
        s.push(startNode);
        cout << "\nDFS 走訪順序: ";
        while (!s.empty()) {
            int v = s.top(); s.pop();
            if (!visited[v]) {
                visited[v] = true; v_ops_dfs++;
                cout << v << " ";
                for (auto& edge : adj[v]) {
                    v_ops_dfs++;
                    if (!visited[edge.first]) s.push(edge.first);
                }
            }
        }
        cout << endl;
    }

    void BFS(int start) {
        fill(visited, visited + n, false);
        queue<int> q;
        visited[start] = true; q.push(start);
        cout << "BFS 走訪順序: ";
        while (!q.empty()) {
            v_space_bfs = max(v_space_bfs, (long long)q.size());
            int v = q.front(); q.pop();
            v_ops_bfs++;
            cout << v << " ";
            for (auto& edge : adj[v]) {
                v_ops_bfs++;
                if (!visited[edge.first]) {
                    visited[edge.first] = true;
                    q.push(edge.first);
                }
            }
        }
        cout << endl;
    }
    void Kruskal() {
        cout << "\nKruskal MST 邊清單:" << endl;
        priority_queue<Edge, vector<Edge>, greater<Edge>> pq;


        for (auto& e_item : all_edges) {
            pq.push(e_item);
            v_ops_kruskal++;
        }
        v_space_kruskal = pq.size();

        DisjointSet ds(n);
        int remain_edges = n - 1;

        while (remain_edges > 0 && !pq.empty()) {
            Edge e_edge = pq.top();
            pq.pop();
            v_ops_kruskal++;

            int g1 = ds.find(e_edge.u, v_ops_kruskal);
            int g2 = ds.find(e_edge.v, v_ops_kruskal);

            if (g1 != g2) {
                ds.parent[g1] = g2; // 合併群組          
                remain_edges--;
                v_ops_kruskal++; // 合併動作
                cout << "  (" << e_edge.u << "," << e_edge.v << ") cost: " << e_edge.weight << endl;
            }
        }
    }

    void Prim() {
        cout << "\nPrim MST 邊清單:" << endl;
        vector<bool> inTV(n, false);
        priority_queue<Edge, vector<Edge>, greater<Edge>> pq;
        inTV[0] = true;
        for (auto& edge : adj[0]) { pq.push({ 0, edge.first, edge.second }); v_ops_prim++; }
        int count = 0;
        while (!pq.empty() && count < n - 1) {
            v_space_prim = max(v_space_prim, (long long)pq.size());
            Edge e_edge = pq.top(); pq.pop();
            v_ops_prim++;
            if (inTV[e_edge.u] && inTV[e_edge.v]) continue;
            int nextV = inTV[e_edge.u] ? e_edge.v : e_edge.u;
            cout << "  (" << e_edge.u << "," << e_edge.v << ") cost: " << e_edge.weight << endl;
            inTV[nextV] = true; count++;
            for (auto& neighbor : adj[nextV]) {
                if (!inTV[neighbor.first]) {
                    pq.push({ nextV, neighbor.first, neighbor.second });
                    v_ops_prim++;
                }
            }
        }
    }

    void Biconnected() {
        num = 1; fill(dfn, dfn + n, 0); fill(low, low + n, 0);
        while (!s_edges.empty()) s_edges.pop();
        cout << "\n--- 雙連通元件分析 ---" << endl;
        Biconnected_Workhorse(0, -1);
    }

    void Biconnected_Workhorse(const int u, const int v) {
        v_ops_bicon++;
        dfn[u] = low[u] = num++;
        for (auto& neighbor : adj[u]) {
            v_ops_bicon++;
            int w = neighbor.first;
            if (v != w && dfn[w] < dfn[u]) {
                s_edges.push({ u, w, 0 });
                v_space_bicon = max(v_space_bicon, (long long)s_edges.size());
            }
            if (dfn[w] == 0) {
                Biconnected_Workhorse(w, u);
                low[u] = min(low[u], low[w]);
                if (low[w] >= dfn[u]) {
                    cout << "找到元件:" << endl;
                    Edge edge;
                    do {
                        v_ops_bicon++;
                        edge = s_edges.top(); s_edges.pop();
                        cout << "  (" << edge.u << "," << edge.v << ")" << endl;
                    } while (!(edge.u == u && edge.v == w));
                }
            }
            else if (w != v) low[u] = min(low[u], dfn[w]);
        }
    }

    void PrintPerformanceTable() {
        cout << "\n" << string(95, '=') << endl;
        cout << left << setw(15) << "演算法項目" << setw(20) << "時間複雜度" << setw(20) << "空間複雜度" << setw(20) << "操作次數" << setw(20) << "空間單位" << endl;
        cout << string(95, '-') << endl;
        cout << setw(15) << "DFS" << setw(20) << "O(n + e)" << setw(20) << "O(n)" << setw(20) << v_ops_dfs << setw(20) << n << endl;
        cout << setw(15) << "BFS" << setw(20) << "O(n + e)" << setw(20) << "O(n)" << setw(20) << v_ops_bfs << setw(20) << v_space_bfs << endl;
        cout << setw(15) << "Kruskal" << setw(20) << "O(e log e)" << setw(20) << "O(e)" << setw(20) << v_ops_kruskal << setw(20) << v_space_kruskal << endl;
        cout << setw(15) << "Prim" << setw(20) << "O(e log e)" << setw(20) << "O(e)" << setw(20) << v_ops_prim << setw(20) << v_space_prim << endl;
        cout << setw(15) << "Biconnected" << setw(20) << "O(n + e)" << setw(20) << "O(e)" << setw(20) << v_ops_bicon << setw(20) << v_space_bicon << endl;
        cout << string(95, '=') << endl;
    }
};

int main() {
    Graph g(7);
    g.InsertEdge(0, 1, 28); g.InsertEdge(0, 5, 10);
    g.InsertEdge(1, 2, 16); g.InsertEdge(1, 6, 14);
    g.InsertEdge(2, 3, 12); g.InsertEdge(3, 4, 22);
    g.InsertEdge(3, 6, 18); g.InsertEdge(4, 5, 25);
    g.InsertEdge(4, 6, 24);

    g.DFS(0); g.BFS(0); g.Kruskal(); g.Prim(); g.Biconnected();
    g.PrintPerformanceTable();

    return 0;
}