#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

void step(std::string s) {
    std::cout << s << "\n";
}

int main() {
    while (true) {
        std::condition_variable cv;
        std::mutex mtx;
        std::atomic<int> stepId(1);
        std::atomic<bool> stop(false);
        auto lambda = [&cv, &mtx, &stepId, &stop](int id) {
            while (true) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&stepId, &id](){
                    return stepId == id;
                });

                step((id == 1) ? "left" : "right");
                stepId.store(3 - id);
                cv.notify_one();
                if (stop.load()) {
                    return;
                }
            }
        };

        std::thread t1(lambda, 1);
        std::thread t2(lambda, 2);

        std::this_thread::sleep_for(std::chrono::microseconds(100));
        stop.store(true);

        t1.join();
        t2.join();

        std::cout << "_________" << std::endl;
    }
    return 0;
}
