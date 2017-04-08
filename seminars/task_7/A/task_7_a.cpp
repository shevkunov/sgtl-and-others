#include <iostream>
#include <thread>

#include "lock_free_queue.h"

#include <cassert>

int main() {
    int stage = 194;
    while(stage--) {
        lock_free_queue<int> lfq;
        std::atomic<size_t> size(0);
        for (int i = 0; i < 10; ++i) {
            lfq.enqueue(i);
        }
        for (int i = 0; i < 10; ++i) {
            int v = 0;
            lfq.dequeue(v);
            assert(v == i);
        }
        auto lambda = [&lfq, &size](int v) {
            for (size_t i = 0; i < 1000; ++i) {
                if ((rand() % 2) || (size > 100)) {
                    lfq.enqueue(v);
                    ++size;
                } else {
                    int get;
                    --size;
                    lfq.dequeue(get);

                }
            }
        };
        std::thread t1(lambda, 1);
        std::thread t2(lambda, 2);
        std::thread t3(lambda, 3);
        std::thread t4(lambda, 4);

        t1.join();
        t2.join();
        t3.join();
        t4.join();
    }

    return 0;
}
