#include <atomic>
#include <memory>
#include <cassert>
#include <atomic>
#include <cassert>


unsigned const MAX_HAZARD_POINTERS = 100;

struct HazardPointer {
    std::atomic<std::thread::id> id;
    std::atomic<void*> pointer;
};

HazardPointer g_hazardPointers[MAX_HAZARD_POINTERS];

class HPOwner {
public:
    HPOwner(const HPOwner&) = delete;
    HPOwner operator=(const HPOwner&) = delete;

    HPOwner()
        : hp_(nullptr) {
        for (unsigned i = 0; i < MAX_HAZARD_POINTERS; ++i) {
            std::thread::id oldId;
            if (g_hazardPointers[i].id.compare_exchange_strong(
                    oldId, std::this_thread::get_id())) {
                hp_ = &g_hazardPointers[i];
                break;
            }
        }
        if (!hp_) {
            throw std::runtime_error("No hazard pointers available");
        }
    }

    std::atomic<void*>& getPointer() {
        return hp_->pointer;
    }

    ~HPOwner() {
        hp_->pointer.store(nullptr);
        hp_->id.store(std::thread::id());
    }

private:
    HazardPointer* hp_;
};

std::atomic<void*>& getHazardPointerForCurrentThread() {
    thread_local static HPOwner hazard;
    return hazard.getPointer();
}

bool outstandingHazardPointersFor(void* p) {
    for (unsigned i = 0; i < MAX_HAZARD_POINTERS; ++i) {
        if (g_hazardPointers[i].pointer.load() == p) {
            return true;
        }
    }
    return false;
}

template<typename T>
void doDelete(void* p) {
    delete static_cast<T*>(p);
}

struct DataToReclaim {
    void* data;
    std::function<void(void*)> deleter;
    DataToReclaim* next;

    template<typename T>
    DataToReclaim(T* p)
        : data(p)
        , deleter(&doDelete<T>)
        , next(nullptr)
    {}

    ~DataToReclaim() {
        deleter(data);
    }
};

std::atomic<DataToReclaim*> nodesToReclaim;

void addToReclaimList(DataToReclaim* node) {
    node->next = nodesToReclaim.load();
    while (!nodesToReclaim.compare_exchange_weak(node->next, node));
}

template<typename T>
void reclaimLater(T* data) {
    addToReclaimList(new DataToReclaim(data));
}

void deleteNodesWithNoHazards() {
    DataToReclaim* current = nodesToReclaim.exchange(nullptr);
    while (current) {
        DataToReclaim* const next = current->next;
        if (!outstandingHazardPointersFor(current->data)) {
            delete current;
        } else {
            addToReclaimList(current);
        }
        current = next;
    }
}

template <class T>
class lock_free_queue { // Well, if you want to write in_snake_case, I will..
public:
    lock_free_queue() : threads_(0) {
        real_head_ = tail_ = head_ = new node;
    }

    ~lock_free_queue() {
        // if somebody works with this queue
        // AFTER destructor call, it is their problem
        ++threads_;
        while (head_ != tail_) {
            T tmp;
            dequeue(tmp);
        }

        deleteNodesWithNoHazards();
        delete head_;
        --threads_;
    }

    void enqueue(T item) {
        ++threads_;

        node* new_node = new node(item);
        node* current_tail;
        node* current_tail_next;

        std::atomic<void*>& hp_current_tail =
                getHazardPointerForCurrentThread();


        while(true) {
            do {
                current_tail = tail_.load();
                hp_current_tail.store(current_tail);
            } while (current_tail != tail_.load());
            current_tail_next = current_tail->next.load();

            if (current_tail == tail_) {
                if (current_tail_next == nullptr) {
                    // in x86 compare_exchange_strong is equal to
                    // compare_exchange_weak
                    if (tail_.load()->next.compare_exchange_strong(
                            current_tail_next, new_node)) {
                        break;
                    }
                } else {
                    tail_.compare_exchange_weak(current_tail,
                            current_tail_next); // helping
                }
            }
        }

        tail_.compare_exchange_weak(current_tail, new_node);
        hp_current_tail.store(nullptr); // TODO CHECK
        --threads_;
    }

    bool dequeue(T& item) {
        ++threads_;
        std::atomic<void*>& hp_current_head =
                getHazardPointerForCurrentThread();
        std::atomic<void*>& hp_current_head_next =
                getHazardPointerForCurrentThread();

        while (true) {
            node* current_head;
            node* current_tail;
            node* current_head_next;

            do {
                do {
                    current_head = head_.load();
                    hp_current_head.store(current_head);
                } while (current_head != head_.load());
                current_head_next = current_head->next.load();
                hp_current_head_next.store(current_head_next);
            } while (current_head != head_.load());
            /// TODO OLD } while (current_head_next != current_head->next.load());
            /// TODO NEW } while (current_head != head_.load());

            if (current_head_next == nullptr) {
                --threads_;
                hp_current_head_next.store(nullptr);
                hp_current_head.store(nullptr);
                return false;
            }

            current_tail = tail_;

            if (current_head == head_) {
                if (current_head == current_tail) {
                    tail_.compare_exchange_weak(current_head,
                            current_head_next); // helping
                } else {
                    if (head_.compare_exchange_strong(current_head,
                                current_head_next)) {
                        item = current_head_next->item;
                        hp_current_head_next.store(nullptr);
                        hp_current_head.store(nullptr);

                        if (outstandingHazardPointersFor(current_head)) {
                            reclaimLater(current_head);
                        } else {
                            delete current_head;
                        }
                        deleteNodesWithNoHazards();
                        --threads_;

                        return true;
                    }
                }
            }
        }
    }

private:
    struct node {
        node() : next(nullptr) {
        }

        explicit node(const T& item) : node(item, nullptr) {
        }

        node(const T& item, node* next) :
            item(item), next(next) {
        }

        T item;
        std::atomic<node*> next;
    };

    std::atomic<unsigned> threads_;
    std::atomic<node*> real_head_;

    std::atomic<node*> head_;
    std::atomic<node*> tail_;

};

