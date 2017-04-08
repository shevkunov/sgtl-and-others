#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>

class blocking_flag {
public:
    blocking_flag()
        : ready_(false)
    {}

    void wait() {
        std::unique_lock<std::mutex> lock(mtx_);
        while (!ready_.load()) {
            ready_cond_.wait(lock);
        }
    }

    void set() {
        //std::unique_lock<std::mutex> lock(mtx_);
        ready_.store(true);
        ready_cond_.notify_all();
    }

private:
    std::atomic<bool> ready_;
    std::mutex mtx_;
    std::condition_variable ready_cond_;
};

int main() {
    std::this_thread::sleep_for(std::chrono::microseconds(17));
    while (true) {
        blocking_flag f;

        std::thread t1(
            [&f]() {
                f.wait();
                std::cout << "1: ready!" << std::endl;
            }
        );

        std::thread t2(
            [&f]() {
                f.wait();
                std::cout << "2: ready!" << std::endl;
            }
        );
        std::thread t3(
            [&f]() {
                f.wait();
                std::cout << "3: ready!" << std::endl;
            }
        );
        f.set();


        t1.join();
        t2.join();
        t3.join();
        std::cout << "_________" << std::endl;
    }
    return 0;
}
