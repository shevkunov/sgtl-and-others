#pragma once
#include <algorithm>
#include <atomic>
 int fetch_and_set_max(std::atomic<int>& x, int y) {
     int mx;
     int xx;
     do {
         mx = std::max(x.load(),y);
         xx = x.load();
     }while (!x.compare_exchange_strong(xx, mx));
    return mx;
 }
