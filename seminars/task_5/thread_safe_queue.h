#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

template<class Value, class Container = std::deque<Value>>
class thread_safe_queue {   // we should write ThreadSafeQueue ...
public:
    explicit thread_safe_queue(size_t capacity) : capacity_(capacity) {
    }

    void enqueue(Value v)
    {
        std::unique_lock<std::mutex> uLock(mutex_);
        if (finished_) {
            throw std::exception();
        }
        cvWrite_.wait(uLock, [&]() { return (queue_.size() < capacity_) || finished_; });
        if ((queue_.size() >= capacity_) || (finished_)) {
            throw std::exception();
        }
        queue_.push_back(std::move(v));
        cvRead_.notify_one();
    }

    void pop(Value& v)
    {
        std::unique_lock<std::mutex> uLock(mutex_);
        if (finished_ && queue_.empty()) {
            throw std::exception();
        }
        cvRead_.wait(uLock, [&]() { return !queue_.empty() || finished_; });
        if (queue_.empty()) {
            throw std::exception();
        }
        v = std::move(queue_.front());
        queue_.pop_front();
        cvWrite_.notify_one();
    }

    void shutdown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        cvRead_.notify_all();
        cvWrite_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cvRead_;
    std::condition_variable cvWrite_;

    Container queue_;
    const size_t capacity_;
    bool finished_ = false;
};

