#include <iostream>

#include <array>
#include <vector>

#include <atomic>
#include <thread>

class PetersonMutex
{
public:
    PetersonMutex()
    {
        want_[0].store(false);
        want_[1].store(false);
        victim_.store(0);
    }
    void lock(int pid)
    {
        want_[pid].store(true);
        victim_.store(pid);
        while (want_[1 - pid].load() && (victim_.load() == pid)) {
            std::this_thread::yield();
        }
    }
    void unlock(int pid)
    {
        want_[pid].store(false);
    }
private:
    std::array<std::atomic<bool>, 2> want_;
    std::atomic<int> victim_;
protected:
};

// Bad codestyle : ignored :: name hardcoded
// in yandex contest (I can't use treeMutex name)
class tree_mutex
{
public:
    tree_mutex(std::size_t numThreads) : numThreads_(numThreads)
    {
        if (numThreads == 0) {
            throw std::runtime_error("numThreads equals zero");
        }
        size_t leafs = 1;
        while (leafs < numThreads) {
            leafs <<= 1;
        }
        mutexCount_ = leafs - 1;
        tree_.resize(mutexCount_);
        for (size_t i = 0; i < mutexCount_; ++i) {
            tree_[i] = new PetersonMutex;
        }
    }

    tree_mutex(tree_mutex &copy) = delete; //Disallow copying

    ~tree_mutex()
    {
        for (size_t i = 0; i < tree_.size(); ++i) {
            delete tree_[i];
        }
    }

    void lock(std::size_t threadIndex)
    {
        if (threadIndex >= numThreads_) {
            throw std::runtime_error("incorrect threadIndex");
        }
        int ptr = threadIndex + mutexCount_;
        while (ptr != 0) {
            int godFather = (ptr - 1) / 2;
            int areWeRightSon = (ptr - 1) % 2;  // 1 - right, 0 - left
            tree_[godFather]->lock(areWeRightSon);
            ptr = godFather;
        }
    }

    void unlock(std::size_t threadIndex)
    {
        if (threadIndex >= numThreads_) {
            throw std::runtime_error("incorrect threadIndex");
        }
        size_t treeIndex = 0;
        size_t leafs = mutexCount_ + 1;
        while (leafs >= 2) {
            if (threadIndex < (leafs >> 1)) {
                // Left Son
                tree_[treeIndex]->unlock(0);
                treeIndex = treeIndex * 2 + 1;
                leafs >>= 1;
            } else {
                // Right Son
                tree_[treeIndex]->unlock(1);
                treeIndex = (treeIndex + 1) * 2;
                leafs >>= 1;
                threadIndex -= leafs;
            }
        }
    }

private:
    const std::size_t numThreads_;
    std::size_t mutexCount_;
    std::vector<PetersonMutex*> tree_; // PetersonMutex is uncopyable
protected:
};
