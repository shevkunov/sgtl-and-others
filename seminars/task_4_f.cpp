#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

#define mp(a, b) make_pair(a, b)
typedef long long int int64;
const int INF = 1E9;

int n, m, e;

int L, R, F;
vector <pair<int,int>> can;

vector < vector<int> > g, gr;
vector <int> mingr;
vector<int> mt, mtr;
vector<char> used, usedr;

bool try_kuhn (int v) {
    if (used[v])  return false;
    used[v] = true;
    for (size_t i = 0; (i < g[v].size()); ++i) {
        int to = g[v][i];
        if (mt[to] == -1 || try_kuhn (mt[to])) {
            mt[to] = v;
            mtr[v] = to;
            return true;
        }
    }
    return false;
}


bool try_kuhnr (int v) {
    if (usedr[v])  return false;
    usedr[v] = true;
    for (size_t i = mingr[v]; (i < gr[v].size()) && (gr[v][i] < R); ++i) {
        int to = gr[v][i];
        if ((L <= to)) {
            if (mtr[to] == -1 || try_kuhnr (mtr[to])) {
                mtr[to] = v;
                mt[v] = to;
                return true;
            }
        } else {
            mingr[v] = i + 1;
        }
    }
    return false;
}

int main() {
    cin >> n >> m >> e;
    g.resize(n);
    gr.resize(m);
    mingr.resize(m, 0);

    for (int i = 0; i < e; ++i) {
        int fr, to;
        cin >> fr >> to;
        --fr; --to;
        g[fr].push_back(to);
        gr[to].push_back(fr);
    }

    for (int i = 0; i < g.size(); ++i) {
        sort(g[i].begin(), g[i].end());
    }

    for (int i = 0; i < gr.size(); ++i) {
        sort(gr[i].begin(), gr[i].end());
    }

    L = 0;
    R = 0;
    F = 0;


    used.assign(n, false);
    usedr.assign(m, false);
    mt.assign (m, -1);
    mtr.resize(n, -1);
    while ((R < n) && (F < m)) {
        if (try_kuhn(R)) {
            used.assign(n, false);
            ++F;
        }
        ++R;
    }
    if (F == m) {
        can.push_back(mp(L,R));
    }

    while (R < n) {
        if (F == m) {
            //if (false && try_kuhn(L)) {
            //    used.assign(n, false);
            //} else {

            if (mtr[L] != -1) {
                int girl = mtr[L];
                mt[mtr[L]] = -1;
                used[L] = false;
                mtr[L] = -1;
                --F;
                ++L;
                if (try_kuhnr(girl)) {
                    usedr.assign(m, false);
                    used.assign(n, false);
                    ++F;
                }
            } else {
                ++L;
            }
        } else {
            if (try_kuhn(R)) {
                usedr.assign(m, false);
                used.assign(n, false);
                ++F;
            }
            ++R;
        }
        if (F == m) {
            can.push_back(mp(L, R));
        }
    }

    while ((R == n) && (F == m)) {
        if (F == m) {
            //if (false && try_kuhn(L)) {
            //    used.assign(n, false);
            //} else {

            if (mtr[L] != -1) {
                int girl = mtr[L];
                mt[mtr[L]] = -1;
                used[L] = false;
                mtr[L] = -1;
                --F;
                ++L;
                if (try_kuhnr(girl)) {
                    usedr.assign(m, false);
                    used.assign(n, false);
                    ++F;
                }
            } else {
                ++L;
            }
        }
        if (F == m) {
            can.push_back(mp(L, R));
        }
    }


    can.push_back(mp(n + 1, n + 1));
    int64 ans = 0;
    for (int i = 0; i + 1 < can.size(); ++i) {
        int64 L = can[i].first;
        int64 R = can[i].second;
        int64 B = can[i + 1].second;
        ans += (L + 1) * (B - R);
    }

    cout << ans;
    return 0;
}
