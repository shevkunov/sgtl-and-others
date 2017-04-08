/*#include <iostream>
#include <thread>
#include <atomic>

class spinlock{
public:
    spinlock() : locked(false) {
    }
    void lock() {
        while (locked.exchange(true)) {
            // wait
        }
    }
    void unlock() {
        locked.store(false);
    }
private:
    std::atomic<bool> locked;
};



int main() {
    return 0;
}
*/
#include <iostream>
#include <thread>
#include <atomic>

using namespace std;

void increment(std::atomic<int> *x) {
    for (int i = 0; i < 10000; ++i) {
        ++*x;
    }
}

int main()
{
    std::atomic<int>  x(0);
    std::thread t1(increment, &x);
    std::thread t2(increment, &x);
    t1.join();
    t2.join();
    cout << "Hello World!" << x.load() << endl;
    return 0;
}
