#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class barrier { /// foo-style. class Barrier expected
public:
    explicit barrier(size_t numThreads) : waiting_(0), exiting_(0), stage_(0), threads_(numThreads) {
        /// explicit Barrier(size_t numThreads) expected
    }

    barrier(barrier &copy) = delete;

    ~barrier() {
        if (waiting_ != 0) {
            throw std::runtime_error("barrier broken");
        }
    }

    void enter() {
        std::unique_lock<std::mutex> lock(mtx_);
        size_t myStage = stage_;
        waiting_++;
        if (waiting_ == threads_) {
            exiting_ = waiting_;
            waiting_ = 0;
            stage_ = !stage_;
            cv_.notify_all();
        } else {
            while (!myStage != stage_) {
                cv_.wait(lock);
            }
        }
    }

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    size_t waiting_;
    size_t exiting_;
    bool stage_;
    const size_t threads_;
};
