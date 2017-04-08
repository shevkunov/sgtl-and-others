//
// Created by randan on 12/9/16.
//

#pragma once

#include <deque>
#include <mutex>
#include <exception>
#include "semaphore.h"

//class semaphore {
//public:
//    explicit semaphore(int initial);
//    void aquire();
//    void release();
//};

template <class Value, class Container = std::deque<Value>>
class thread_safe_queue {
public:
    explicit thread_safe_queue(size_t capacity)
        :size_(0)
        ,slots_(capacity)
    {}

    void enqueue(Value&& v){
        std::unique_lock<std::mutex>(lock_);
        if (downed_)
            throw std::exception();
        slots_.aquire();
        if (downed_)
            throw std::exception();
        queue_.push_back(std::move(v));
        ++current_size_;
        size_.release();

    }

    void pop(Value& v){
        std::unique_lock<std::mutex>(lock_);
        if(downed_ && current_size_ == 0)
            throw std::exception();
        size_.aquire();
        if(downed_ && current_size_ == 0) {
            slots_.release();
            throw std::exception();
        }
        v = std::move(queue_.front());
        queue_.pop_front();
        --current_size_;
        slots_.release();
    }

    void shutdown() {
        std::unique_lock<std::mutex>(lock_);
        downed_ = true;
        slots_.release();
        size_.release();
    }
private:
    Container queue_;
    semaphore size_;
    semaphore slots_;
    std::mutex lock_;
    bool downed_ = false;
    std::size_t current_size_ = 0;
};
