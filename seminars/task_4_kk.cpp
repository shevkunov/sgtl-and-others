#include <vector>
#include <queue>
#include <iostream>
#include <algorithm>

using std::cin;
using std::cout;

const int INF = 2*1E9;

struct Edge
{
    int to, reverseEdge;
    int flow, capacity;
    int cost, mark;
    Edge(int to, int reverseEdge, int capacity, int cost) :
        to(to), reverseEdge(reverseEdge), flow(0), capacity(capacity), cost(cost), mark(0) {
    }
    Edge(int next, int revEdge, int cap, int cost, int mark) :
        to(next), reverseEdge(revEdge), flow(0), capacity(cap), cost(cost), mark(mark) {
    }
};

struct Graph
{
    int n;
    std::vector<std::vector<Edge>> edge;
    std::vector<int> p, dist, flags, from;
    std::priority_queue< std::pair<int, int> > qq;
    Graph(int n) : n(n), edge(n), p(n), dist(n), flags(n), from(n) {
    }
    void pushEdge(int a, int b, int cap, int cost, int mark) {
        edge[a].push_back(Edge(b, edge[b].size(), cap, cost, mark));
        edge[b].push_back(Edge(a, edge[a].size() - 1, 0, -cost, mark));
    }
    std::pair<int, int> minCostMaxFlow(int S, int T) {
        p.assign(p.size(), INF);
        p[S] = 0;
        bool pushed = true;
        while (pushed) {
            pushed = false;
            for (int j = 0; j < n; ++j) {
                if (p[j] < INF) {
                    for (int k = 0; k < edge[j].size(); ++k) {
                        if (edge[j][k].capacity > 0 && p[edge[j][k].to] > p[j] + edge[j][k].cost) {
                            p[edge[j][k].to] = p[j] + edge[j][k].cost;
                            pushed = true;
                        }
                    }
                }
            }
        }
        int resultFlow = 0, resultCost = 0;
        while (true) {
            flags.assign(flags.size(), 0);
            dist.assign(dist.size(), INF);
            dist[S] = 0;
            qq.push(std::make_pair(0, S));
            while (!qq.empty()) {
                int i = qq.top().second;
                int d = 0 - qq.top().first;
                qq.pop();

                if (dist[i] == d) {
                    flags[i] = 1;
                    for (size_t j = 0; j < edge[i].size(); ++j) {
                        if (edge[i][j].capacity > edge[i][j].flow) {
                            int to = edge[i][j].to;
                            int tmp = dist[i] + edge[i][j].cost + p[i] - p[to];
                            if (!flags[to] && (dist[to] > tmp)) {
                                dist[to] = tmp;
                                from[to] = edge[i][j].reverseEdge;
                                qq.push(std::make_pair(0 - tmp, to));
                            }
                        }
                    }
                }
            }
            if (!flags[T]) {
                break;
            }
            for (int i = 0; i < n; ++i) {
                if (flags[i]) {
                    p[i] += dist[i];
                } else {
                    p[i] += dist[T];
                }
            }
            int pushFlow = INF, pushCost = 0;
            for (int i = T; i != S; ) {
                int to = edge[i][from[i]].to;
                int re = edge[i][from[i]].reverseEdge;
                pushFlow = std::min(pushFlow, edge[to][re].capacity - edge[to][re].flow);
                pushCost += edge[to][re].cost;
                i = to;
            }
            for (int i = T; i != S; ) {
                int to = edge[i][from[i]].to;
                int re = edge[i][from[i]].reverseEdge;
                edge[to][re].flow += pushFlow;
                edge[i][from[i]].flow -= pushFlow;
                i = to;
            }
            resultFlow += pushFlow;
            resultCost += pushFlow * pushCost;
        }
        return std::make_pair(resultFlow, resultCost);
    }
};


int main() {
    int n, m, e;
    cin >> n >> m >> e;
    int S = n + m;
    int T = n + m + 1;
    Graph g(n + m + 2);
    for (int i = 0; i < n; ++i) {
        int v1;
        cin >> v1;
        g.pushEdge(S, i, 1, 0 - v1, - 1);
    }
    for (int i = 0; i < m; ++i) {
        int v2;
        cin >> v2;
        g.pushEdge(n + i, T, 1, 0 - v2, -1);
    }
    for (int i = 0; i < e; ++i) {
        int fr, to;
        cin >> fr >> to;
        --fr;
        --to;
        g.pushEdge(fr, n + to, 1, 0, i + 1);
    }

    std::pair<int, int> flowCost = g.minCostMaxFlow(S, T);
    std::vector<int> ans;
    for (size_t i = 0; i < n; ++i) {
        for (int j = 0; j < g.edge[i].size(); ++j) {
            if ((n <= g.edge[i][j].to) && (g.edge[i][j].to < n + m) && (g.edge[i][j].flow)) {
                ans.push_back(g.edge[i][j].mark);
            }
        }
    }
    cout << 0 - flowCost.second << "\n" << ans.size() << "\n";
    for (int i : ans) {
        cout << i << " ";
    }
    return 0;
}
