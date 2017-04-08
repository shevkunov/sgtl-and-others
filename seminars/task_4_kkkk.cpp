#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

#define mp(a, b) make_pair(a, b)
const int INF = 1E9;
int n1, n2, e;

struct edge{
    int from;
    int to;
    int id;
    int flow;
    int cap;
    edge() {
    }
    edge(int from, int to, int id, int flow, int cap)
        : from(from), to(to), id(id), flow(flow), cap(cap) {
    }
};

vector < vector<int> > g;
vector <edge> elist;
vector <int> p;

vector<int> was;
vector<int> used;
vector<int> spW;

vector<pair<int, int>> order;
int main() {
    cin >> n1 >> n2 >> e;
    p.resize(n1 + n2 + 2);
    g.resize(n1 + n2 + 2);
    order.resize(n1);
    int S = n1 + n2;
    int T = n1 + n2 + 1;
    for (int i = 0; i < n1; ++i) {
        cin >> p[i];
        order[i] = mp(0 - p[i], i);
    }

    sort(order.begin(), order.end());

    for (int i = n1; i < n2 + n1; ++i) {
        cin >> p[i];
    }

    for (int i = 0; i < e; ++i) {
        int fr, to;
        cin >> fr >> to;
        --fr; --to;
        to += n1;
        g[fr].push_back(elist.size());
        elist.push_back((edge(fr, to, i, 0, 1)));
        g[to].push_back(elist.size());
        elist.push_back((edge(to, fr, -i, 0, 0)));
    }

    used.resize(g.size());
    for (int i = 0; i < order.size();) {
        was.assign(g.size(), -1);
        queue<pair<int, int>> q;
        int j;
        for (j = i; (j < order.size()) && (p[order[i].second] == p[order[j].second]); ++j) {
            was[order[j].second] = -2;
            q.push(mp(order[j].second,order[j].second));
        }
        bool found = true;
        spW.clear();
        while (found) {
            for (int i = 0; i < g.size(); ++i) {
                if (was[i] > -2) {
                    was[i] = -1;
                }
            }
            for (int i : spW) {
                was[i] = -3;
            }
            for (j = i; (j < order.size()) && (p[order[i].second] == p[order[j].second]); ++j) {
                if (was[order[j].second] != -3)
                q.push(mp(order[j].second,order[j].second));
            }
            found = false;
            int maxValueBfsed = 0 - INF;
            int maxVertex = 0;
            int maxGodFather = 0;
            while (!q.empty()) {
                int v = q.front().first;
                int gF = q.front().second;
                q.pop();
                for (size_t i = 0; i < g[v].size(); ++i) {
                    int to = elist[g[v][i]].to;
                    if ((elist[g[v][i]].flow < elist[g[v][i]].cap) && ((was[to] == -1) || (was[to] == -3))) {
                        was[to] = g[v][i];
                        q.push(mp(to, gF));
                        if ((!used[to]) && (to >= n1) && (p[to] > maxValueBfsed)) {
                            maxValueBfsed = p[to];
                            maxVertex = to;
                            maxGodFather = gF;
                        }
                    }
                }
            }

            if (maxValueBfsed != 0 - INF) {
                was[maxGodFather] = -3;
                spW.push_back(maxGodFather);
                found = true;
                used[maxVertex] = true;
                int v = maxVertex;
                while (was[v] >= 0) {
                    elist[was[v]].flow += 1;
                    elist[was[v] ^ 1].flow -= 1;
                    v = elist[was[v]].from;
                }
            }
        }

        i = j;
    }

    int cost = 0;
    vector <int> ans;
    for (edge e : elist) {
        if ((e.flow == e.cap) && (e.from < n1)) {
            ans.push_back(e.id);
            cost += p[e.from] + p[e.to];
        }
    }

    cout << cost << "\n" << ans.size() << "\n";
    for (int i : ans) {
        cout << i + 1 << " ";
    }
    return 0;
}
