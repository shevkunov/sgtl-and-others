#pragma once
#include <algorithm>
#include <atomic>
#include <condition_variable>

//one more try
class recursive_mutex {
public:
    recursive_mutex() : rec_(0), locked_id_(0) {
    }

    void lock() {
        std::unique_lock<std::mutex> lock(mx_);
        while (true) {
            if ((locked_id_ == std::this_thread::get_id()) ||
                (rec_ == 0)) {
                locked_id_ = std::this_thread::get_id();
                ++rec_;
                break;
            }
            cv_.wait(lock);
        }
    }

    void unlock() {
        std::unique_lock<std::mutex> lock(mx_);
        rec_ = std::max(rec_ - 1, 0);
        cv_.notify_all();
    }

private:
    volatile int rec_;
    std::thread::id locked_id_;
    std::mutex mx_;
    std::condition_variable cv_;
};
