

## 1. 解題說明

### 問題描述
實作一個圖形類別（Graph Class），並整合以下五大功能：
1.  **圖形遍歷**：深度優先搜尋 (DFS) 與 廣度優先搜尋 (BFS)。
2.  **最小生成樹 (MST)**：找出權重總和最小且連結所有頂點的樹，分別使用 Kruskal 與 Prim 演算法。
3.  **雙連通元件 (Biconnected Components)**：識別圖中的雙連通元件。
4.  **效能驗證**：透過程式計數器統計操作次數與空間佔用，驗證理論複雜度。

### 解題策略
*   **資料結構**：使用 **鄰接串列 (Adjacency List)** 儲存圖形。
*   **演算法選擇**：
    *   **Kruskal**：使用 `Disjoint Set` (併查集)檢測圖是否相連 。
    *   **Prim**：實作 Min-Heap，每次選取最小邊。
    *   **Biconnected**：利用 DFS 產生的發現時間（dfn）與回溯值（low）來判斷關節點。
---

## 2. 程式實作內容
### 一. 基礎結構定義
*   **Edge (邊結構)：**
    儲存起點 `u`、終點 `v` 與權重 `weight`。透過**運算子重載**（`<` 與 `>`），讓演算法（如 Kruskal 的排序與 Prim 的優先佇列）能自動依照權重比較路徑成本。
*   **DisjointSet (併查集)：**
    Kruskal 演算法的核心工具。
    *   **find：** 遞迴尋找群組的「根節點」，用來判斷兩個是否連通。
    *   **unite：** 將兩個不相連的群組合併。

### 二. Graph 類別私有成員
*   **adj (鄰接串列)：** 使用 `vector` 陣列儲存，只記錄有連通的路徑。
*   **dfn & low：** `dfn` 紀錄進入順序，`low` 紀錄該點能回溯到的最老祖先。
*   **v_ops & v_space：** 負責統計每次運算的邏輯操作次數與記憶體，用來對照理論上的效能。


### 三、 演算法運作邏輯

#### (1). 圖形遍歷 (Traversal)
*   **DFS (深度優先)：** 
    採用`stack`(堆疊)。
*   **BFS (廣度優先)：** 
    使用 `queue` (佇列)。

#### (2). 最小生成樹 (MST)
*   **Kruskal 演算法：** 
    把圖上所有路**從短排到長**，接著由最短的路開始選，只要不造成迴路就選取該路徑。
*   **Prim 演算法：** 
    從起點開始，每次都在目前路徑相連路徑中找一條**最短的路**連向新點，直到所有點都連通。

#### (3). 雙連通元件 (Biconnected Components)
利用 DFS 過程中產生的` dfn`  與 ` low` ，找出圖形中的「雙連通元件」。
```cpp
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
```



---

## 3. 效能分析

| 演算法 | 時間複雜度 | 空間複雜度 | 
| :--- | :--- | :--- | 
| **DFS** | $O(n + e)$ | $O(n)$ | 
| **BFS** | $O(n + e)$ | $O(n)$ | 
| **Kruskal** | $O(e \log e)$ | $O(e)$ | 
| **Prim** | $O(e \log e)$ | $O(e)$ 
| **Biconnected** | $O(n + e)$ | $O(e)$ 

---

## 4. 測試與驗證

### 輸出結果(n=7 e=9)
```text
DFS 走訪順序: 0 5 4 6 3 2 1
BFS 走訪順序: 0 1 5 2 6 4 3

Kruskal MST 邊清單:
  (0,5) cost: 10
  (2,3) cost: 12
  (1,6) cost: 14
  (1,2) cost: 16
  (3,4) cost: 22
  (4,5) cost: 25

Prim MST 邊清單:
  (0,5) cost: 10
  (5,4) cost: 25
  (4,3) cost: 22
  (3,2) cost: 12
  (2,1) cost: 16
  (1,6) cost: 14

--- 雙連通元件分析 ---
找到元件:
  (6,3)
  (6,1)
  (4,6)
  (5,0)
  (4,5)
  (3,4)
  (2,3)
  (1,2)
  (0,1)

===============================================================================================
演算法項目     時間複雜度          空間複雜度          操作次數            空間單位
-----------------------------------------------------------------------------------------------
DFS            O(n + e)            O(n)                25                  7
BFS            O(n + e)            O(n)                25                  3
Kruskal        O(e log e)          O(e)                44                  9
Prim           O(e log e)          O(e)                15                  4
Biconnected    O(n + e)            O(e)                34                  9
===============================================================================================
```

---

## 5. 申論及開發報告
### 為什麼「理論複雜度」和「實際操作次數」會有差？

#### (1) DFS / BFS

* 理論：$O(n + e)$
* 實際：25 次（$n=7, e=9$）

**說明：**
理論上就是每個點、每條邊看過一次而已。但這次是無向圖，所以一條邊會被看兩次（A→B、B→A）。

所以實際算起來會變成：
7（點）+ 2×9（邊）= 25

---

#### (2) Kruskal

* 理論：$O(e \log e)$
* 實際：44 次

**說明：**

`find` 會多跑幾層，所以操作次數變多。

---

#### (3) Prim

* 理論：$O(e \log e)$
* 實際：15 次

**說明：**
Prim 是從一個點慢慢長出去，每次選最小的邊。

---

#### (4) Biconnected

* 理論：$O(n + e)$
* 實際：34 次

**說明：**
這個也是和 DFS一樣，但多做一件事：把邊丟進 stack，再拿出來。所以每條邊除了走訪，還多了 push / pop。
也因為這樣，實際次數會比普通 DFS 多一點。

---
