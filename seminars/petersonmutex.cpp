#include <iostream>

#include <array>
#include <vector>

#include <atomic>
#include <thread>

#include "tree_mutex.h"


template <class Mutex>
void incrementor(int *x, Mutex *mutex, int pid)
{
    for (int i = 0; i < 10000; ++i) {
        mutex->lock(pid);
        ++*x;
        mutex->unlock(pid);
    }
}

int main()
{
    int x = 0;
    PetersonMutex petersonMutex;
    std::thread p1(incrementor<PetersonMutex>, &x, &petersonMutex, 0);
    std::thread p2(incrementor<PetersonMutex>, &x, &petersonMutex, 1);
    p1.join();
    p2.join();
    std::cout << "Peterson Summary = " << x << std::endl;

    x = 0;
    tree_mutex treeMutex(5);
    std::thread t1(incrementor<tree_mutex>, &x, &treeMutex, 0);
    std::thread t2(incrementor<tree_mutex>, &x, &treeMutex, 1);
    std::thread t3(incrementor<tree_mutex>, &x, &treeMutex, 2);
    std::thread t4(incrementor<tree_mutex>, &x, &treeMutex, 3);
    std::thread t5(incrementor<tree_mutex>, &x, &treeMutex, 4);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    std::cout << "Tree Summary = " << x << std::endl;
    return 0;
}

