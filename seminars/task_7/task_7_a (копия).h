#include <atomic>
template <class T>
class lock_free_queue {
public:
    lock_free_queue() {
        tail = head = new node;
    }

    ~lock_free_queue() {
        /*todelete*/
    }

    void enqueue(T item) {
        node* new_node = new node(item);
        node* current_tail;
        node* current_tail_next;

        while(true) {
            current_tail = tail;
            current_tail_next = tail.load()->next; // is he still alive ?
            
            if (current_tail_next == nullptr) {
                // in x86 compare_exchange_strong is equal to
                // compare_exchange_weak
                if (tail.load()->next.compare_exchange_strong(current_tail_next,
                        new_node)) {
                    break;
                }
            } else {
                tail.compare_exchange_weak(current_tail,
                        current_tail_next); // helping
            }
        }

        tail.compare_exchange_weak(current_tail, new_node);
    }

    bool dequeue(T& item) {
        while (true) {
            node* current_head = head;
            node* current_tail = tail;
            node* current_head_next = current_head->next; // is he still alive ?
            
            if (current_head == current_tail) {
                if (current_head_next == nullptr) {
                    return false;
                } else {
                    tail.compare_exchange_weak(current_head,
                            current_head_next); // helping
                }
            } else {
                if (head.compare_exchange_strong(current_head,
                            current_head_next)) {
                    item = current_head_next->item;
                    return true;
                }
            }
        }
        // delete head;
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

    std::atomic<node*> realhead;
    std::atomic<node*> head;
    std::atomic<node*> tail;
};
