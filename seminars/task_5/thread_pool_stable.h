#pragma once
#include <mutex>
#include <queue>
#include <future>
#include <functional>


template <class Value>
class thread_pool {
public:
    explicit thread_pool(size_t num_workers) : working_(true) {
        threads_.resize(num_workers);
        for (size_t i = 0; i < num_workers; ++i) {
            threads_[i] = std::thread([&](){
                std::unique_lock<std::mutex> lock(mutex_);

                while(true) {                 
                   cv_.wait(lock, [&](){ return (func_queue_.size() > 0) || !working_; });

                   while (func_queue_.size() > 0) {
                       std::function<Value()> function = func_queue_.front();
                       func_queue_.pop();
                       std::promise<Value> promise = std::move(prom_queue_.front());
                       prom_queue_.pop();

                       lock.unlock();
                       try {
                           promise.set_value(function());
                       } catch (std::exception&) {
                           promise.set_exception(std::current_exception());
                       }
                       lock.lock();
                   }

                   if (!working_) {
                       return;
                   }
                }
            });
        }
    }

    std::future<Value> submit(std::function<Value()> f) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!working_) {
            throw std::runtime_error("thread_pool :: already shutdown");
        }
        func_queue_.push(f);
        prom_queue_.push(std::promise<Value>());
        cv_.notify_one();
        return prom_queue_.back().get_future();
    }

    void shutdown(){
        {
            std::lock_guard<std::mutex> lock(mutex_);
            working_ = false;
            cv_.notify_all();
        }

        for (size_t i = 0; i < threads_.size(); ++i) {
            threads_[i].join();
        }
    }

private:
    bool working_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::thread> threads_;
    std::queue<std::function<Value()>> func_queue_;
    std::queue<std::promise<Value>> prom_queue_;

};
