#pragma once
#include <cassert>
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
            threads_[i] = std::thread([&]() {
                std::unique_lock<std::mutex> lock(mutex_);
                while(true) {
                    cv_.wait_for(lock, std::chrono::nanoseconds(0), [&]() {
                        return (func_queue_.size() > 0) || !working_;
                    });
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

                        cv_.notify_all();
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
        std::future<Value> rf = prom_queue_.back().get_future();
        cv_.notify_one();
        return rf;
    }

    Value wait(std::future<Value> future) {
        std::unique_lock<std::mutex> lock(mutex_);
        while (true) {
            if (!((func_queue_.size() > 0) || futureDone(std::ref(future)) || !working_)) {
                cv_.wait_for(lock,std::chrono::nanoseconds(0), [&](){
                    return (func_queue_.size() > 0) || futureDone(std::ref(future)) || !working_;
                });
            }
            if (futureDone(std::ref(future))) {
                return future.get();
            }
            if (!working_) {
                throw std::runtime_error("thread_pool::wait failed");
            }
            if (func_queue_.size() > 0) {
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
                cv_.notify_all();
            }

        }
    }

    void shutdown(){
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!working_) {
                return;
            }
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
    static inline bool futureDone(std::future<Value>& future) {
        return future.wait_for(std::chrono::nanoseconds(0)) != std::future_status::timeout;
    }
};


class parallel_sort {
public:
    explicit parallel_sort(size_t thread_count, size_t sort_barrier = 1024, size_t sort_manually = 4)
        : pool_(thread_count), SORT_BARRIER(sort_barrier), SORT_MANUALLY(sort_manually) {
        assert(SORT_MANUALLY >= 1);
        assert(SORT_BARRIER >= 1);
    }
    template <class RandomAccessIterator, class Compare>
    void sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp) {
        using VT = typename RandomAccessIterator::value_type;
        std::vector<VT> array; // That iterators so slow!
        for (RandomAccessIterator ii = first; ii != last; ++ii) {
            array.push_back(std::move(*ii));
        }
        std::vector<VT> buffer(array.size());
        std::future<bool> f = pool_.submit(std::bind<bool>(&parallel_sort::template parallel_sorter<VT, Compare>,
                                         this, std::ref(array), std::ref(buffer), 0, array.size(), std::ref(comp)));
        (void)(f.get());
        int i = 0;
        for (RandomAccessIterator ii = first; ii != last; ++ii, ++i) {
            *ii = std::move(array[i]);
        }
    }
private:
    thread_pool<bool> pool_;
    const size_t SORT_BARRIER;
    const size_t SORT_MANUALLY;
    template <class VT, class Compare>
    bool parallel_sorter(std::vector<VT> &array,std::vector<VT> &buffer, size_t left, size_t right, Compare& less) {
        size_t len = right - left;

        if (right <= left) {
            return true;
        }

        if (len > SORT_BARRIER) {
            size_t center = (left + right) / 2;
            std::future<bool> f = pool_.submit(std::bind<bool>(&parallel_sort::template parallel_sorter<VT, Compare>,
                                             this, std::ref(array), std::ref(buffer), center, right, std::ref(less)));
            parallel_sorter(array, buffer, left, center, less);
            (void)(pool_.wait(std::move(f)));
            merge(array, buffer, left, center, right, less);
        } else {
            single_sorter(array, buffer, left, right, less);
        }
        return true;
    }

    template <class VT, class Compare>
    bool single_sorter(std::vector<VT> &array,std::vector<VT> &buffer, size_t left, size_t right, Compare& less) {
        if (right - left <= SORT_MANUALLY) {
            bool changed = true;
            while (changed) {
                changed = false;
                for (size_t i = left; i + 1 < right; ++i) {
                    if (less(array[i + 1], array[i])) {
                        std::swap(array[i], array[i + 1]);
                        changed = true;
                    }
                }
            }
        } else {
            size_t center = (right + left) / 2;
            single_sorter(array, buffer, left, center, less);
            single_sorter(array, buffer, center, right, less);
            merge(array, buffer, left, center, right, less);
        }
        return true;
    }

    template <class T, class Compare>
    void merge(std::vector<T> &array, std::vector<T> &buffer, size_t left, size_t center, size_t right, Compare &less) {
        size_t pL = left;
        size_t pR = center;
        int i = left;
        while ((pL != center) || (pR != right)) {
            if ((pL != center) && ((pR == right) || (less(array[pL], array[pR])))) {
                buffer[i++] = std::move(array[pL]);
                ++pL;
            } else {
                buffer[i++] = std::move(array[pR]);
                ++pR;
            }
        }

        for (size_t i = left; i < right; ++i) {
            array[i] = std::move(buffer[i]);
        }
    }
};
