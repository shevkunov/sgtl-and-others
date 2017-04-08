#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

#include "parallel_sort.h"

int main() {
    size_t t, l;
    std::vector <int> v[2];
    std::cin >> t >> l;

    for (size_t i = 0; i < l; ++i) {
        int val = rand();
        v[0].push_back(val);
        v[1].push_back(val);
    }

    parallel_sort ps(t);

    for (t = 0; t < 1 ;++t) {
        std::random_shuffle(++v[0].begin(), v[0].end()--);
        std::random_shuffle(++v[1].begin(), v[1].end()--);
        std::sort(++v[0].begin(), v[0].end()--, [](int a, int b) { return a < b; });
        time_t ltime = time(NULL);
         ps.sort(++v[1].begin(), v[1].end()--, [](int a, int b) { return a < b; });
        std::cout << time(NULL) - ltime << "\n";
        for (size_t i = 0; i < l ; ++i) {
            if (v[0][i] != v[1][i]) {
                std::cout << "FAIL!\n";
                return 0;
            }
        }

        std::cout << "OK\n";
    }
    return 0;
}
