#include <iostream>
#include <vector>
#include <thread>


#include "./exam/recursive_mutex.h";




int main() {
    std::recursive_mutex mx;
    for (int i = 0; i < 100; ++i) {
    recursive_mutex mx;
    std::thread t1([&mx](){
        mx.lock();
        mx.lock();
        mx.unlock();
        mx.unlock();
        mx.unlock();
    });
    std::thread t2([&mx](){
        mx.lock();
        mx.lock();
        mx.unlock();
        mx.unlock();
        mx.unlock();
    });
    std::thread t3([&mx](){
        mx.lock();
        mx.lock();
        mx.unlock();
        mx.unlock();
    });
    mx.lock();
    mx.lock();
    mx.unlock();
    mx.unlock();
    t1.join();
    t2.join();
    t3.join();
    }
}
