#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <unistd.h>
#include <sys/sem.h>

/** From seminar lesson */
class Semaphore {
public:
    Semaphore() : counter_(0) {
    }
    void post() {
        std::unique_lock<std::mutex> lock(m_);
        ++counter_;
        cv_.notify_one();
    }
    void wait() {
        std::unique_lock<std::mutex> lock(m_);
        cv_.wait(lock, [&]() { return counter_ > 0;});
        --counter_;
    }
private:
    int counter_;
    std::condition_variable cv_;
    std::mutex m_;
};

void step(std::string s) {
    std::cout << s << "\n";
}

int main() {
    while(true) {
        std::atomic<bool> stop(false);
        Semaphore sem[2];

        auto lambda = [&sem, &stop](int id) {
            while (true) {
                sem[id - 1].wait();
                if (stop.load()) {
                    return;
                }
                step((id == 1) ? "left" : "right");
                sem[3 - id - 1].post();
            }
        };

        sem[0].post();

        std::thread t1(lambda, 1);
        std::thread t2(lambda, 2);

        std::this_thread::sleep_for(std::chrono::microseconds(100));

        stop.store(true);
        sem[0].post();
        sem[1].post();

        t1.join();
        t2.join();

        std::cout << "_________" << std::endl;
    }
    return 0;
}

