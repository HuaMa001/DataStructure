#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <stack>
#include <string>

using namespace std;

// 1. 定義統一的邊結構[cite: 2, 5]
struct Edge {
    int u, v, weight;
    // Kruskal 排序用 (由小到大)[cite: 5]
    bool operator<(const Edge& o) const {
        return weight < o.weight;
    }
    // Prim 的 Min-Heap 優先權佇列用[cite: 2]
    bool operator>(const Edge& o) const {
        return weight > o.weight;
    }
};

// 2. 併查集實作，用於 Kruskal 偵測迴路[cite: 5]
struct DisjointSet {
    vector<int> parent;
    DisjointSet(int n) {
        parent.resize(n, -1);
    }
    int find(int i) {
        if (parent[i] < 0) return i;
        return parent[i] = find(parent[i]); // 路徑壓縮
    }
    void unite(int i, int j) {
        int root1 = find(i);
        int root2 = find(j);
        if (root1 != root2) parent[root1] = root2;
    }
};

class Graph {
private:
    int n;           // 頂點數
    int e;           // 邊數
    int num;         // dfn 序號[cite: 4]
    int* dfn;        // 發現序號[cite: 4]
    int* low;        // 回溯值[cite: 4]
    bool* visited;   // 遍歷標記

    // 儲存結構
    vector<pair<int, int>>* adj; // 相鄰串列: {鄰點, 權重}
    vector<Edge> all_edges;      // 邊集合 (供 Kruskal 使用)
    stack<Edge> s;               // 邊堆疊 (供 Biconnected 使用)[cite: 4]

public:
    Graph(int nodes) : n(nodes), e(0), num(0) {
        adj = new vector<pair<int, int>>[n];
        dfn = new int[n];
        low = new int[n];
        visited = new bool[n];
    }

    ~Graph() {
        delete[] adj;
        delete[] dfn;
        delete[] low;
        delete[] visited;
    }

    // 插入帶權重的邊
    void InsertEdge(int u, int v, int w = 1) {
        if (u >= 0 && u < n && v >= 0 && v < n) {
            adj[u].push_back({ v, w });
            adj[v].push_back({ u, w }); // 無向圖
            all_edges.push_back({ u, v, w });
            e++;
        }
    }

    // --- 基礎搜尋 ---
    void DFS(int v) {
        cout << "DFS 順序: ";
        fill(visited, visited + n, false);
        DFS_Workhorse(v);
        cout << endl;
    }

    void DFS_Workhorse(int v) {
        visited[v] = true;
        cout << v << " ";
        for (auto& edge : adj[v]) {
            if (!visited[edge.first]) DFS_Workhorse(edge.first);
        }
    }

    void BFS(int start) {
        fill(visited, visited + n, false);
        queue<int> q;
        visited[start] = true;
        q.push(start);
        cout << "BFS 順序: ";
        while (!q.empty()) {
            int v = q.front(); q.pop();
            cout << v << " ";
            for (auto& edge : adj[v]) {
                if (!visited[edge.first]) {
                    visited[edge.first] = true;
                    q.push(edge.first);
                }
            }
        }
        cout << endl;
    }

    // --- 雙連通元件 (Biconnected Components)[cite: 4] ---
    void Biconnected() {
        num = 1;
        fill(dfn, dfn + n, 0);
        fill(low, low + n, 0);
        while (!s.empty()) s.pop();
        cout << "\n--- Biconnected Components 分析 ---" << endl;
        Biconnected_Workhorse(0, -1);
    }

    void Biconnected_Workhorse(const int u, const int v) {
        dfn[u] = low[u] = num++;
        for (auto& neighbor : adj[u]) {
            int w = neighbor.first;
            if (v != w && dfn[w] < dfn[u]) {
                s.push({ u, w, 0 }); // 權重在此不重要
            }
            if (dfn[w] == 0) {
                Biconnected_Workhorse(w, u);
                low[u] = min(low[u], low[w]);
                if (low[w] >= dfn[u]) {
                    cout << "找到元件:" << endl;
                    Edge edge;
                    do {
                        edge = s.top(); s.pop();
                        cout << "  (" << edge.u << "," << edge.v << ")" << endl;
                    } while (!(edge.u == u && edge.v == w));
                }
            }
            else if (w != v) {
                low[u] = min(low[u], dfn[w]);
            }
        }
    }

    // --- Kruskal's Algorithm[cite: 2, 5] ---
    void Kruskal() {
        vector<Edge> T;
        sort(all_edges.begin(), all_edges.end()); // 1. 選擇花費最低的邊[cite: 5]
        DisjointSet ds(n);

        int i = 0;
        // 2. 當樹邊少於 n-1 且仍有邊時[cite: 2]
        while (T.size() < n - 1 && i < all_edges.size()) {
            Edge e = all_edges[i++];
            // 3. 檢查是否形成迴路[cite: 5]
            if (ds.find(e.u) != ds.find(e.v)) {
                ds.unite(e.u, e.v);
                T.push_back(e);
            }
        }

        if (T.size() < n - 1) cout << "no spanning tree" << endl;
        else {
            cout << "\nKruskal MST 邊清單:" << endl;
            for (auto& e : T) cout << "  (" << e.u << "," << e.v << ") cost: " << e.weight << endl;
        }
    }

    // --- Prim's Algorithm[cite: 2, 3] ---
    void Prim() {
        vector<Edge> T;
        vector<bool> inTV(n, false);
        priority_queue<Edge, vector<Edge>, greater<Edge>> pq; // Min-Heap[cite: 2]

        inTV[0] = true; // 從頂點 0 開始[cite: 2, 3]
        for (auto& edge : adj[0]) {
            pq.push({ 0, edge.first, edge.second });
        }

        while (T.size() < n - 1 && !pq.empty()) {
            Edge e = pq.top(); pq.pop();
            if (inTV[e.u] && inTV[e.v]) continue; // 避免迴路

            int nextV = inTV[e.u] ? e.v : e.u;
            T.push_back(e);
            inTV[nextV] = true;

            for (auto& neighbor : adj[nextV]) {
                if (!inTV[neighbor.first]) {
                    pq.push({ nextV, neighbor.first, neighbor.second });
                }
            }
        }

        if (T.size() < n - 1) cout << "no spanning tree" << endl;
        else {
            cout << "\nPrim MST 邊清單:" << endl;
            for (auto& e : T) cout << "  (" << e.u << "," << e.v << ") cost: " << e.weight << endl;
        }
    }
};

int main() {
    // 建立教材範例中的圖[cite: 3, 5]
    Graph g(7);
    g.InsertEdge(0, 1, 28);
    g.InsertEdge(0, 5, 10);
    g.InsertEdge(1, 2, 16);
    g.InsertEdge(1, 6, 14);
    g.InsertEdge(2, 3, 12);
    g.InsertEdge(3, 4, 22);
    g.InsertEdge(3, 6, 18);
    g.InsertEdge(4, 5, 25);
    g.InsertEdge(4, 6, 24);

    g.DFS(0);
    g.BFS(0);
    g.Kruskal();
    g.Prim();
    g.Biconnected();

    return 0;
}