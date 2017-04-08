#pragma once
#include <queue>
#include <future>

template <class Value>
class thread_pool {
public:
    explicit thread_pool(size_t num_workers) {

    }

    std::future<Value> submit(std::function<Value>) {

    }

    void shutdown(){

    }

private:
    bool working_;
    std::queue<std::function<Value>> queue_;
};
