#pragma once
#include <algorithm>
#include <atomic>
#include <condition_variable>

class sorted_list {
public:
    sorted_list () : root(0) {
    }

    void insert( int x ) {
        mx_.lock();
        if (root == 0) {
            root = new node ();
            root->v = x;
        } else {
            node* lst = root;
            node* nxt = root->next;

            while (nxt || (nxt->v < x)) {
                lst
            }


        }

    }

    bool erase( int x );
    bool has( int x ) const;

private:
    struct node {
        int v;
        node* next;
        std::mutex mx_;
        node() : next(0) {
        }
    };
    std::mutex mx_;
    node* root;
};
