#include <iostream>
#include <vector>

#define int64 long long int
std::vector <std::vector<int64>> graph;
std::vector<int64> potentialU;
std::vector<int64> potentialV;
std::vector<int64> pairs;
std::vector<int64> ways;
std::vector<int64> edges;

int main() {
    int64 n, nSize;

    std::cin >> n;
    nSize = n + 1;
    graph.resize(nSize, std::vector<int64>(nSize));
    for (int64 i = 1; i <= n; ++ i) {
        for (int64 j = 1; j <= n; ++j) {
            std::cin >> graph[i][j];
        }
    }
    potentialU.resize(nSize);
    potentialV.resize(nSize);
    pairs.resize(nSize);
    ways.resize(nSize);

    for (int64 i = 1; i <= n; ++i) {
        pairs[0] = i;
        int64 jj = 0;

        std::vector<int64> plusMin(nSize, 1E9);
        std::vector<char> usd(nSize, false);

        do {
            usd[jj] = true;
            int64 diff = 1E9;
            int64 jjj;
            for (int64 j = 1; j <= n; ++j) {
                if (!usd[j]) {
                    if (graph[pairs[jj]][j] - potentialU[pairs[jj]] - potentialV[j] < plusMin[j]) {
                        plusMin[j] = graph[pairs[jj]][j] - potentialU[pairs[jj]] - potentialV[j];
                        ways[j] = jj;
                    }
                    if (plusMin[j] < diff) {
                        diff = plusMin[j];
                        jjj = j;
                    }
                }
            }
            for (int64 j = 0; j <= n; ++j) {
                if (usd[j]) {
                    potentialU[pairs[j]] += diff;
                    potentialV[j] -= diff;
                } else {
                    plusMin[j] -= diff;
                }
            }
            jj = jjj;
        } while(pairs[jj] != 0);

        do {
            pairs[jj] = pairs[ways[jj]];
            jj = ways[jj];
        } while(jj);
    }


    edges.resize(nSize);
    for (int64 j = 1; j <= n; ++j) {
        edges[pairs[j]] = j;
    }
    int64 cost = 0 - potentialV[0];

    std::cout << cost << "\n";
    for (int64 i = 1; i <= n; ++i) {
        std::cout << i << " "  << edges[i] << "\n";
    }

    return 0;
}
