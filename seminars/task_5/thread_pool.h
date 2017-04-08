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
                    if (!((func_queue_.size() > 0) || !working_)) {
                        cv_.wait_for(lock, std::chrono::nanoseconds(0), [&](){ return (func_queue_.size() > 0) || !working_; });
                    }

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
                       cvWait_.notify_all();
                    }

                    if (!working_) {
                       return;
                    }
                }
            });
        }
    }

    thread_pool(thread_pool& other) = delete;

    ~thread_pool() {
        shutdown();
    }

    std::future<Value> submit(std::function<Value()> f) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!working_) {
            throw std::runtime_error("thread_pool :: already shutdown");
        }
        func_queue_.push(f);
        prom_queue_.push(std::promise<Value>());
        cv_.notify_one();
        cvWait_.notify_one();
        return prom_queue_.back().get_future();
    }

    Value wait(std::future<Value> future) {
        std::unique_lock<std::mutex> lock(mutex_);
        while (future.wait_for(std::chrono::nanoseconds(0)) == std::future_status::timeout) {
            cvWait_.wait_for(lock,std::chrono::nanoseconds(0),[&](){ return (func_queue_.size() > 0)
                        || (future.wait_for(std::chrono::nanoseconds(1)) != std::future_status::timeout); });

            if (future.wait_for(std::chrono::nanoseconds(0)) != std::future_status::timeout) {
                break;
            }

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
                cvWait_.notify_all();
            }

        }
        return future.get();
    }

    void shutdown(){
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!working_) {
                return;
            }
            working_ = false;
            cv_.notify_all();
            cvWait_.notify_all();
        }

        for (size_t i = 0; i < threads_.size(); ++i) {
            threads_[i].join();
        }
    }

private:
    bool working_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable cvWait_;
    std::vector<std::thread> threads_;
    std::queue<std::function<Value()>> func_queue_;
    std::queue<std::promise<Value>> prom_queue_;

};


class parallel_sort {
public:
    explicit parallel_sort(size_t thread_count) : pool_(thread_count) {
    }
    template <class RandomAccessIterator, class Compare>
    void sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp) {
        std::vector<typename RandomAccessIterator::value_type> buffer(last - first);
        std::future<bool> f = pool_.submit([&]() -> bool {
            return sorter(buffer, first, first, last, comp);
        });
        bool b = f.get();
        (void)(b);
    }
private:
    thread_pool<bool> pool_;
    template <class RandomAccessIterator, class Compare>
    bool sorter(std::vector<typename RandomAccessIterator::value_type>& buffer, RandomAccessIterator& first,
                RandomAccessIterator& begin, RandomAccessIterator& end, Compare& less) {
        const size_t SORT_BARRIER = 64;
        size_t len = end - begin;

        if (len > SORT_BARRIER) {
            RandomAccessIterator center = begin + len / 2;
            std::future<bool> f = pool_.submit([&]() -> bool {
                return sorter(buffer, first, center, end, less);
            });

            sorter(buffer, first, begin, center, less);

            (void)pool_.wait(std::move(f));

            RandomAccessIterator pL = begin, pR = center;

            int i = begin - first;
            while ((pL != center) || (pR != end)) {
                if ((pL != center) && ((pR == end) || (less(*pL, *pR)))) {
                    buffer[i] = *pL;
                    ++pL;
                } else {
                    buffer[i] = *pR;
                    ++pR;
                }
                ++i;
            }

            i = begin - first;
            for (RandomAccessIterator ii = begin; ii != end; ++i, ++ii) {
                *ii = buffer[i];
            }

            return true;
        } else {
            std::sort(begin, end, less);
            return true;
        }

    }
};
