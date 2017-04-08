#include <vector>
#include <queue>
#include <iostream>
#include <algorithm>
#include <cassert>

using std::cin;
using std::cout;

typedef long long int64;

const int64 INF = 2*1E18;
struct Edge
{
    int to, reverseEdge;
    int64 flow, capacity;
    int64 cost, mark;
    Edge(int to, int reverseEdge, int64 capacity, int64 cost) :
        to(to), reverseEdge(reverseEdge), flow(0), capacity(capacity), cost(cost), mark(0) {
    }
    Edge(int next, int revEdge, int64 cap, int64 cost, int64 mark) :
        to(next), reverseEdge(revEdge), flow(0), capacity(cap), cost(cost), mark(mark) {
    }
};

struct Graph
{
    int n;
    std::vector<std::vector<Edge>> edge;
    std::vector<int64> p, dist, flags, from;
    std::priority_queue< std::pair<int64, int64> > qq;
    Graph(int n) : n(n), edge(n), p(n), dist(n), flags(n), from(n) {
    }
    void pushEdge(int a, int b, int64 cap, int64 cost, int64 mark) {
        edge[a].push_back(Edge(b, edge[b].size(), cap, cost, mark));
        edge[b].push_back(Edge(a, edge[a].size() - 1, 0, -cost, mark));
    }
    std::pair<int64, int64> minCostMaxFlow(int S, int T) {
        p.assign(p.size(), INF);
        p[S] = 0;
        bool pushed = true;
        while (pushed) {
            pushed = false;
            for (int j = 0; j < n; ++j) {
                if (p[j] < INF) {
                    for (size_t k = 0; k < edge[j].size(); ++k) {
                        if (edge[j][k].capacity > 0 && p[edge[j][k].to] > p[j] + edge[j][k].cost) {
                            p[edge[j][k].to] = p[j] + edge[j][k].cost;
                            pushed = true;
                        }
                    }
                }
            }
        }
        int64 resultFlow = 0, resultCost = 0;
        while (true) {
            flags.assign(flags.size(), 0);
            dist.assign(dist.size(), INF);
            dist[S] = 0;
            qq.push(std::make_pair(0, S));
            while (!qq.empty()) {
                int i = qq.top().second;
                int64 d = 0 - qq.top().first;
                qq.pop();

                if (dist[i] == d) {
                    flags[i] = 1;
                    for (size_t j = 0; j < edge[i].size(); ++j) {
                        if (edge[i][j].capacity > edge[i][j].flow) {
                            int to = edge[i][j].to;
                            int64 tmp = dist[i] + edge[i][j].cost + p[i] - p[to];
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
            int64 pushFlow = INF, pushCost = 0;
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

using std::vector;
using std::cin;
using std::cout;

const int64 BIG = 1E9;

int P, F, G;

int getBoy(int i) {
    assert((0 <= i) && (i < F));
    return P * G * 2  + P + i;
}

int getGirlA(int g, int p) {
    assert((0 <= g) && (g < G));
    assert((0 <= p) && (p < P));
    return (G * p + g)*2;
}


int getGirlB(int g, int p) {
    assert((0 <= g) && (g < G));
    assert((0 <= p) && (p < P));
    return (G * p + g)*2 + 1;
}

int getMed(int p) {
    assert((0 <= p) && (p <= P));
    return G * P*2 + p;
}

vector<int> q;
vector<vector<int>> photo;

vector<vector<int>> girlIncoming;


void fill(int v, int mark, Graph& g) {
    for (size_t i = 0; i < g.edge[v].size(); ++i) {
        if ((g.edge[v][i].flow > 0) && (g.edge[v][i].capacity > 0)) {
            --g.edge[v][i].flow;
            g.edge[v][i].mark = mark;
            fill(g.edge[v][i].to, mark, g);
            return;
        }
    }
}

int main() {
    cin >> P >> F >> G;
    girlIncoming.resize(G);
    q.resize(G);
    for (size_t i = 0; i < q.size(); ++i) {
        cin >> q[i];
    }

    photo.resize(P);
    for (int p = 0; p < P; ++p) {
        int size;
        cin >> size;
        photo[p].resize(size);
        for (int i = 0; i < size; ++i) {
            cin >> photo[p][i];
            --photo[p][i];
            girlIncoming[photo[p][i]].push_back(p);
        }
    }

    if (G <= F) {
        cout << "0\n";
        for (size_t p = 0; p < photo.size(); ++p) {
            for (size_t i = 0; i < photo[p].size(); ++i) {
                cout << photo[p][i] + 1 << " ";
            }
            cout << "\n";
        }
    } else {
        Graph g(G*P*2 + F + P + 2);
        int addedBig = 0;
        int S = G*P*2 + F + P;
        int T = G*P*2 + F + P + 1;
        for (int i = 0; i < F; ++i) {
            g.pushEdge(S, getBoy(i), 1, 0, 0);
            for (int j = 0; j < G; ++j) {
                if (girlIncoming[j].size())
                g.pushEdge(getBoy(i), getGirlA(j, girlIncoming[j][0]), 1, 0, 0);
            }
        }
        for (int i = 0; i < P; ++i) {
            if (i + 1 < P) {
                g.pushEdge(getMed(i), getMed(i + 1), 100, 0, 0);
            } else {
                g.pushEdge(getMed(i), T, 100, 0, 0);
            }
            for (size_t j = 0; j < photo[i].size(); ++j) {
                g.pushEdge(getMed(i), getGirlA(photo[i][j], i), 100, q[photo[i][j]], 0);
                ++addedBig;
                g.pushEdge(getGirlA(photo[i][j], i), getGirlB(photo[i][j], i), 1, -BIG, 0);
                g.pushEdge(getGirlA(photo[i][j], i), getGirlB(photo[i][j], i), 100, 0, 0);
                if (i + 1 < P) {
                    g.pushEdge(getGirlB(photo[i][j], i), getMed(i + 1), 100, 0, 0);
                } else {
                    g.pushEdge(getGirlB(photo[i][j], i), T, 100, 0, 0);
                }
            }
        }
        for (int i = 0; i < G; ++i) {
            for (size_t j = 0; j + 1 < girlIncoming[i].size(); ++j) {
                g.pushEdge(getGirlB(i,girlIncoming[i][j]), getGirlA(i, girlIncoming[i][j+1]), 100, 0, 0);
            }
        }

        std::pair<int64, int64> r = g.minCostMaxFlow(S, T);
        int64 cost = r.second + BIG * addedBig;

        for (int i = 0; i < F; ++i) {
            fill(getBoy(i), i + 1, g);
        }
        cout << cost << "\n";

        for (int i1 = 0; i1 < photo.size(); ++i1) {
            for (int i2 = 0; i2 < photo[i1].size(); ++i2) {
                int i = getGirlA(photo[i1][i2], i1);
                for (size_t j = 0; j < g.edge[i].size(); ++j) {
                    if ((g.edge[i][j].capacity == 1) && (g.edge[i][j].cost == -BIG)) {
                        cout << g.edge[i][j].mark << " ";
                    }
                }
            }
            cout << "\n";
        }
    }


    return 0;
}
