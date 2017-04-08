#include <atomic>
#include <cassert>

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
        tryRelease();
        delete head_;
        --threads_;
    }

    void enqueue(T item) {
        ++threads_;

        node* new_node = new node(item);
        node* current_tail;
        node* current_tail_next;

        while(true) {
            current_tail = tail_;
            current_tail_next = tail_.load()->next; // is he still alive ?
            
            if (current_tail_next == nullptr) {
                // in x86 compare_exchange_strong is equal to
                // compare_exchange_weak
                if (tail_.load()->next.compare_exchange_strong(current_tail_next,
                        new_node)) {
                    break;
                }
            } else {
                tail_.compare_exchange_weak(current_tail,
                        current_tail_next); // helping
            }
        }

        tail_.compare_exchange_weak(current_tail, new_node);

        tryRelease();
        --threads_;
    }

    bool dequeue(T& item) {
        ++threads_;

        while (true) {
            node* current_head = head_;
            node* current_tail = tail_;
            node* current_head_next = current_head->next; // is he still alive ?
            
            if (current_head == current_tail) {
                if (current_head_next == nullptr) {

                    tryRelease();
                    --threads_;
                    return false;
                } else {
                    tail_.compare_exchange_weak(current_head,
                            current_head_next); // helping
                }
            } else {
                if (head_.compare_exchange_strong(current_head,
                            current_head_next)) {
                    item = current_head_next->item;

                    tryRelease();
                    --threads_;
                    return true;
                }
            }
        }
    }

private:
    struct node {
        node() : next(nullptr) {
        }

        explicit node(T item) : node(item, nullptr) {
        }

        node(T item, node* next) :
            item(item), next(next) {
        }

        T item;
        std::atomic<node*> next;
    };

    std::atomic<unsigned> threads_;
    std::atomic<node*> real_head_;

    std::atomic<node*> head_;
    std::atomic<node*> tail_;

    void try_release() {
        node* old_head = head_;
        node* temp = real_head_;
        if (threads_.load() == 1) {
            while (real_head_ != old_head) {
                temp = real_head_;
                real_head_.store(temp->next);
                delete temp;
            }
        }
    }
};
