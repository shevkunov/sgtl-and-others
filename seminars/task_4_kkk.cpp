#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int n, k, e;
vector < vector<int> > g;
vector <pair<int, int>> elist;
vector<int> mt;
vector<char> used;

vector <int> v1;
vector <int> v2;

vector <int> mx;

vector <pair<pair<int, int>,int>> order;

int cost = 0;
int try_kuhn (int v) {
    if (used[v])  return 0;
    used[v] = true;
    int mxx = 0;
    for (size_t i=0; i<g[v].size(); ++i) {
        int to = g[v][i];
        if (mt[to] == -1) {
            if (v2[to] >= 0) {
               // mt[to] = v;
                mxx = max(mxx, v2[to]);
            }
        } else {
            int ret = try_kuhn (mt[to]);
            if (ret >= 0) {
               // mt[to] = v;
                mxx =  max(ret, mxx);;
            }
        }
    }
    return mxx;
}

int do_kuhn (int v, int mxx) {
    if (used[v])  return 0;
    used[v] = true;
    for (size_t i=0; i<g[v].size(); ++i) {
        int to = g[v][i];
        if (mt[to] == -1) {
            if ((v2[to] >= 0) && (mxx == v2[to])) {
                mt[to] = v;
                return mxx;
            }
        } else {
            int ret = do_kuhn (mt[to], mxx);
            if (ret == mxx) {
                mt[to] = v;
                return mxx;
            }
        }
    }
    return 0;
}

int main() {
    cin >> n >> k >> e;
    v1.resize(n);
    v2.resize(k);
    order.resize(n);
    for (int i = 0; i < n; ++i) {
        cin >> v1[i];
    }
    for (int i = 0; i < k; ++i) {
        cin >> v2[i];
    }
    g.resize(n);
    for (int i = 0; i < e; ++i) {
        pair<int, int> p;
        cin >> p.first >> p.second;
        --p.first;
        --p.second;
        elist.push_back(p);
        g[p.first].push_back(p.second);
    }

    mt.assign(k, -1);

    for (int i = 0; i < n; ++i) {
        used.assign (n, false);
        order[i].first.first = 0 - v1[i] - try_kuhn (i);
        order[i].first.second = 0;
        order[i].second = i;
    }
    std::sort(order.begin(), order.end());

    for (int vv=0; vv < n; ++vv) {
        int v = order[vv].second;
        used.assign (n, false);
        int mxx = try_kuhn (v);
        if (mxx) {
            cost += v1[v] +  mxx;
            used.assign (n, false);
            do_kuhn(v, mxx);
        }
    }

    vector <int> ans;
    for (int i = 0; i < elist.size(); ++i) {
        if (mt[elist[i].second] == elist[i].first) {
            ans.push_back(i);
        }
    }
    cout << cost << "\n";
    cout << ans.size() << "\n";
    for (int i : ans) {
        cout << i + 1 << " ";
    }
    return 0;
}
