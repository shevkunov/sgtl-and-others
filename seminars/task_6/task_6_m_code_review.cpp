#include <iostream>
#include <string>
#include <vector>
#include <ios>
#include <stdexcept>
#include <cctype>


/** I have written huge template library with many algos,
 * where this algo is written by using a templates ...
 *     ... but I think it isn't appropriate here :( */

class SuffixAutomaton {
public:
    SuffixAutomaton(int lowerCharBound = 0, int upperCharBound = 256) :
        lowerCharBound_(lowerCharBound), upperCharBound_(upperCharBound),
        last_(0), state_(1, Node(lowerCharBound, upperCharBound)) {
    }

    struct Node {
    public:
        explicit Node(int lowerCharBound = 0, int upperCharBound = 256) :
            Node(0, lowerCharBound, upperCharBound){
        }

        explicit Node(int length, int lowerCharBound, int upperCharBound) :
            length(length), link(-1),
            next(std::vector<int>(upperCharBound - lowerCharBound, -1)) {
        }

        size_t length;
        int link;
        std::vector<int> next;
    };

    inline void normalizeCharacter(char &c) {
        if (!((lowerCharBound_ <= c) && (c < upperCharBound_))) {
            std::string where = "sgtl::SuffixAutomaton::addChar::";
            std::string what = "bad character";
            throw std::runtime_error(where + what);
        } else {
            c -= lowerCharBound_;
        }
    }

    void addChar(char c) {
        normalizeCharacter(c);
        // names cur, p, q, clone is used in lection and are
        // appropriate here
        int cur = static_cast<int>(state_.size());
        state_.push_back(Node(state_[last_].length + 1,
                              lowerCharBound_, upperCharBound_));
        int p = last_;
        while ((p != -1) && (state_[p].next[c] == -1)) {
            state_[p].next[c] = cur;
            p = state_[p].link;
        }
        if (p == -1) {
            state_[cur].link = 0;
        } else {
            int q = state_[p].next[c];
            if (state_[q].length == state_[p].length + 1) {
                state_[cur].link = q;
            } else {
                int clone = state_.size();
                state_.push_back(Node(state_[q]));
                state_[clone].length = state_[p].length + 1;

                while ((p != -1) && (state_[p].next[c] == q)) {
                    state_[p].next[c] = clone;
                    p = state_[p].link;
                }

                state_[cur].link = clone;
                state_[q].link = clone;
            }
        }

        last_ = cur;
    }

    int next(int stateId, char c) {
        normalizeCharacter(c);
        return state_[stateId].next[c];
    }

    size_t stateLength(int stateId) {
        return state_[stateId].length;
    }

private:
    int lowerCharBound_;
    int upperCharBound_;
    int last_;
    std::vector<Node> state_;
};

class Nenokku {
public:
    explicit Nenokku(int lowerCharBound = 0, int upperCharBound = 256) :
        a(SuffixAutomaton(lowerCharBound, upperCharBound)) {
    }

    void add(const std::string &s) {
        for (size_t i = 0; i < s.length(); ++i) {
            a.addChar(s[i]);
        }
    }

    bool exists(const std::string &s) {
        int root = 0;
        for (size_t i = 0; (i < s.length()) && (root != -1); ++i) {
            root = a.next(root, s[i]);
        }
        return root != -1;
    }

private:
    SuffixAutomaton a;
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    char queryType;
    std::string str;
    Nenokku solver('a', 'z' + 1);
    while (std::cin >> queryType) {
        std::cin >> str;
        for (size_t i = 0; i < str.length(); ++i) {
            str[i] = tolower(str[i]);
        }
        if (queryType == 'A') {
            solver.add(str);
        } else {
            std::cout << (solver.exists(str) ? "YES\n" : "NO\n");
        }
    }
    return 0;
}
