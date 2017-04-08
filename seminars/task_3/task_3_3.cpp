#include <iostream>

#include "barrier.h"

int main() {
    barrier br(3);

    auto lambda = [&br](size_t id) {
        while(true) {
            br.enter();
            std::cout << "Broken : " << id << std::endl;
        }
    };
    std::thread t[3];
    for (int i = 0; i < 3; ++i) {
        t[i] = std::thread(lambda, i);
    }

    for (int i = 0; i < 3; ++i) {
        t[i].join();
    }
    return 0;
}
