#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <ios>
#include <algorithm>
#include <stdexcept>


namespace sgtl {
    typedef long long int hash_t;
    template <hash_t base_ = 256, hash_t module_ = 1000000007>
    class HashedString {
    public:
        HashedString(char* c) : s_(std::string(c)) {
            buildPows_();
            h_ = buildHash(s_);
        }

        HashedString(const std::string& s) : s_(s) {
            buildPows_();
            h_ = buildHash(s_);
        }

        static std::vector<hash_t> buildHash(const std::string& s) {
            std::vector<hash_t> h_(s.length());
            h_[0] = s[0];
            for (size_t i = 1; i < s.length(); ++i) {
                h_[i] = (h_[i - 1] + s[i] * pows_[i]) % module_;
            }
            return h_;
        }

        hash_t getRawHash(size_t l, size_t r) {
            if ((l != r) && (r)) {
                if (!l) {
                    return h_[r - 1];
                } else {
                    return (h_[r - 1] - h_[l - 1] + module_) % module_;
                }
            } else {
                return 0;
            }
        }

        hash_t getRightedHash(size_t l, size_t r) {
            return (getRawHash(l, r) * pows_[h_.size() - r]) % module_;
        }

        hash_t moveHashRight(hash_t hash, size_t dist) {
            return (hash * pows_[dist]) % module_;
        }

        bool compareSubStrings(size_t l, size_t r,
                               HashedString<base_, module_> &hs,
                               size_t sl, size_t sr) {
            if ((r - l) != (sr - sl)) {
                return false;
            } else {
                if (l <= sl) {
                    return (getRawHash(l, r) * pows_[sl - l]) % module_
                            == hs.getRawHash(sl, sr);
                } else {
                    return (hs.getRawHash(sl, sr) * pows_[l - sl]) % module_
                            == getRawHash(l, r);
                }
            }
        }

    private:
        std::string s_;
        std::vector<hash_t> h_;
        static std::vector<hash_t> pows_;

        void buildPows_() {
            size_t old_size = pows_.size();
            if (old_size < s_.size()) {
                pows_.resize(s_.length() + 1);
                pows_[0] = 1;
                for (size_t i = std::max(old_size, (size_t)1); i < pows_.size(); ++i) {
                    pows_[i] = (pows_[i - 1] * base_) % module_;
                }
            }
        }
    };

    template <sgtl::hash_t base_, sgtl::hash_t module_>
    std::vector<sgtl::hash_t> sgtl::HashedString<base_, module_>::pows_;
}

namespace sgtl {
    // @Tested
    /// Vector is something, which has [] and .size(), std::string for example
    template <class Vector> // TODO : len_t like in prefixFunction
    std::vector<size_t> zFunction(const Vector &s) {
        std::vector<size_t> z(s.size());
        z[0] = s.size();
        size_t rightestBound = 0; /// )
        size_t rightestBoundIndex = 0;
        for (size_t i = 1; i < s.size(); ++i) {
            if (i < rightestBound) {
                z[i] = z[i - rightestBoundIndex];
                if (i + z[i] > rightestBound) {
                    z[i] = rightestBound - i;
                }
            }

            while ((i + z[i] < s.size()) && (s[i + z[i]] == s[z[i]])) {
                ++z[i];
            }

            if (i + z[i] > rightestBound) {
                rightestBoundIndex = i;
                rightestBound = i + z[i];
            }
        }
        return z;
    }



    namespace sgtl {
        // @Tested
        std::vector<size_t> zFunctionPure(std::string &s, std::vector<size_t> &z, size_t start = 0) {
            z.assign(s.length(), 0);
            z[start] = s.length() - start;
            size_t rightestBound = start; /// )
            size_t rightestBoundIndex = start;
            for (size_t i = start + 1; i < s.length(); ++i) {
                if (i < rightestBound) {
                    z[i] = z[i - rightestBoundIndex + start];
                    if (i + z[i] > rightestBound) {
                        z[i] = rightestBound - i;
                    }
                }

                while ((i + z[i] < s.length()) && (s[i + z[i]] == s[z[i] + start])) {
                    ++z[i];
                }

                if (i + z[i] > rightestBound) {
                    rightestBoundIndex = i;
                    rightestBound = i + z[i];
                }
            }
            return z;
        }
    }
}

namespace sgtl {
    std::vector<size_t> reversePrefixFunction(const std::vector<size_t>& p) {
        std::vector<size_t> s(1, 0);
        size_t newChar = 0;
        for (size_t i = 1; i < p.size(); ++i) {
            if (p[i] == 0) {
                s.push_back(++newChar);
            } else {
                s.push_back(s[p[i] - 1]);
            }
        }
        return s;
    }
}

namespace sgtl {
    /**  Kasai, Arimura, Arikawa, Lee, Park algorithm.
     * suffix array should be 0-nimerated and correct **/
    std::vector<size_t> buildLCPfromSufArr(const std::string& s, const std::vector<size_t>& sufArr) {
        if (s.size() != sufArr.size()) {
            throw std::runtime_error("sgtl::buildLCPfromSufArr - Bad suffix array");
        }
        std::vector<size_t> lcp(s.size() - 1, 0);
        std::vector<size_t> revSufArr(sufArr.size());
        for (size_t i = 0; i < sufArr.size(); ++i) {
            revSufArr[sufArr[i]] = i;
        }
        size_t k = 0;
        for (size_t i = 0; i < s.size(); ++i) {
            if (k) {
                --k;
            }
            if (revSufArr[i] == s.size() - 1) {
                k = 0;
            } else {
                int next = sufArr[revSufArr[i] + 1];
                while ((std::max(i + k, next + k) < s.size()) && (s[i + k] == s[next+k])) {
                    ++k;
                }
                lcp[revSufArr[i]] = k;
            }
        }
        return lcp;
    }
}

namespace sgtl {
    template <class C, C lo, C hi>
    class PrefixTreeNode {
    public:
        PrefixTreeNode() : leaf_(false), size_(0),
            links_(std::vector<PrefixTreeNode*>(hi - lo + 1, nullptr)) {
        }
        ~PrefixTreeNode() {
            for (PrefixTreeNode* node : links_) {
                if (node != nullptr) {
                    delete node;
                }
            }
        }

        PrefixTreeNode(PrefixTreeNode& other) = delete;

        PrefixTreeNode* go(const C& ch) {
            if (links_[ch - lo] == nullptr) {
                links_[ch - lo] = new PrefixTreeNode();
            }
            return links_[ch - lo];
        }

        PrefixTreeNode* tryGo(const C& ch) {
            return links_[ch - lo];
        }

        void setLeaf() {
            if (!leaf_) {
                ++size_;
            }
            leaf_ = true;
        }

        bool isLeaf() const {
            return leaf_;
        }

        void recalcSize() {
            size_ = isLeaf();
            for (PrefixTreeNode* node : links_) {
                if (node != nullptr) {
                    size_ += node->size();
                }
            }
        }

        size_t size() const {
            return size_;
        }

    private:
        bool leaf_;
        size_t size_;
        std::vector<PrefixTreeNode*> links_;
    };

    template <class C, C lo, C hi>
    class PrefixTree {
    public:
        PrefixTree() : root(new PrefixTreeNode<C, lo, hi>) {
        }

        ~PrefixTree() {
            delete root;
        }

        PrefixTree(PrefixTree& other) = delete;

        template<class Vector>
        void insert(const Vector& s) {
            std::vector<PrefixTreeNode<C, lo, hi>*> ptr;
            ptr.push_back(root);
            for (C ch : s) {
                ptr.push_back(ptr.back()->go(ch));
            }
            ptr.back()->setLeaf();
            while (!ptr.empty()) {
                ptr.back()->recalcSize();
                ptr.pop_back();
            }
        }

        template<class Vector>
        bool check(const Vector& s) {
            PrefixTreeNode<C, lo, hi>* ptr = root;
            for (C ch : s) {
                ptr = ptr->tryGo(ch);
                if (ptr == nullptr) {
                    return false;
                }
            }
            return ptr->isLeaf();
        }

        template<class Vector>
        Vector nth(size_t indx) {
            Vector ans;
            PrefixTreeNode<C, lo, hi>* ptr = root;
            if (indx >= ptr->size()) {
                throw std::runtime_error("sgtl::PrefixTree::nth - Bad index");
            }
            ++indx;
            while (!((indx <= 0) && (ptr->isLeaf()))) {
                C last = lo;
                size_t lastLeftSize = 0;
                size_t leftSize = 0;
                for (C i = lo; i < hi; ++i) {
                    PrefixTreeNode<C, lo, hi>* tryGo = ptr->tryGo(i);
                    if (tryGo != nullptr) {
                        if (leftSize < indx) {
                            last = i;
                            lastLeftSize = leftSize;
                        } else {
                            break;
                        }
                        leftSize += tryGo->size();
                    }
                }
                indx -= lastLeftSize + ptr->tryGo(last)->isLeaf();
                leftSize = 0;
                ptr = ptr->tryGo(last);
                ans.push_back(last);
            }
            return ans;
        }

    private:
        PrefixTreeNode<C, lo, hi>* root;
    };
}

namespace sgtl {
    /// Vector is something, which has [] and .size(), std::string for example
    template <class Vector, class len_t = size_t>
    std::vector<len_t> prefixFunction(const Vector &s) {
        std::vector<len_t> p(s.size());
        len_t k = 0;
        for (size_t i = 1; i < s.length(); ++i) {
            while ((k > 0) && (s[i] != s[k])) {
                k = p[k - 1];
            }
            if (s[i] == s[k]) {
                ++k;
            }
            p[i] = k;
        }
        return p;
    }
}

namespace sgtl {

    struct PalindromicTreeNode {
        explicit PalindromicTreeNode(int length)
            : PalindromicTreeNode(length, nullptr) {
        }

        PalindromicTreeNode(int length, PalindromicTreeNode* sufLink)
            : length(length), occAsMax(0), occ(0), sufLink(sufLink),
              next(std::vector<PalindromicTreeNode*>(256, nullptr)) {
        }

        int length;
        size_t num;
        size_t occAsMax;
        size_t occ;
        PalindromicTreeNode* sufLink;
        std::vector<PalindromicTreeNode*> next;
    };

    // @NotTested (But DualPalindromicTree tested)
    //TODO dynamyc lo and hi like in DualPalindromicTree
    template <class Node = PalindromicTreeNode>
    class PalindromicTree {
    public:
        explicit PalindromicTree(const std::string &s)
            : root1(new Node(-1)), root2(new Node(0)), cntPalindromes(0) {
            garbageCollector.push_back(root1);
            garbageCollector.push_back(root2);
            root1->sufLink = root1;
            root2->sufLink = root1;
            suffix = root2;
            push(s);
            calculateOccs();

        }

        ~PalindromicTree() {
            //TODO :: recursive deleting
            for(size_t i = 0; i < garbageCollector.size(); ++i) {
                delete garbageCollector[i];
            }
        }

        void findAddSuffix(Node* &cur, int pos, int &curLen,
                           const std::string &str) {
            while (true) {
                curLen = cur->length;
                if ((pos - 1 - curLen >= 0) &&
                        (str[pos - 1 - curLen] == str[pos])) {
                    break;
                }
                cur = cur->sufLink;
            }
        }

        void makeSuffixLink(Node* &cur, int pos, int &curLen, int letter,
                            const std::string &str) {
            while (true) {
                cur = cur->sufLink;
                curLen = cur->length;
                if ((pos - 1 - curLen >= 0) &&
                        (str[pos - 1 - curLen] == str[pos])) {
                    suffix->sufLink = cur->next[letter];
                    break;
                }
            }
        }

        void addLetter(int pos, const std::string &str) {
            Node* cur = suffix;
            int letter = str[pos], curLen = 0;
            findAddSuffix(cur, pos, curLen, str);
            if (cur->next[letter] != nullptr) {
                suffix = cur->next[letter];
                ++suffix->occAsMax;
                return;
            }
            suffix = new Node(cur->length + 2);
            ++suffix->occAsMax;
            garbageCollector.push_back(suffix);

            cur->next[letter] = suffix;
            if (suffix->length == 1) {
                suffix->sufLink = root2;
                suffix->num = 1;
                return;
            }
            makeSuffixLink(cur, pos, curLen, letter, str);
            suffix->num = 1 + suffix->sufLink->num;
        }

        void push(const std::string &str) {
            for (size_t i = 0; i < str.length(); ++i) {
                addLetter(i, str);
                cntPalindromes += suffix->num;
            }
        }

        void calculateOccs() {
            std::vector<Node*> order;
            int bound = 0;
            for (int i = garbageCollector.size() - 1; i >= 0; --i) {
                Node *n = garbageCollector[i];
                n->occ = n->occAsMax;
            }
            for (int i = garbageCollector.size() - 1; i >= 0; --i) {
                Node *n = garbageCollector[i];
                n->sufLink->occ += n->occ;
            }
        }

        Node* root1;
        Node* root2;
        Node* suffix;
        size_t cntPalindromes;

        std::vector<Node*> garbageCollector;
    };


    //@Tested
    template <size_t MaxLinks = 256>
    struct DualPalindromicTreeNode {
        explicit DualPalindromicTreeNode(int length)
            : DualPalindromicTreeNode(length, nullptr) {
        }

        DualPalindromicTreeNode(int length, DualPalindromicTreeNode* sufLink)
            : length(length), sufLink(sufLink),
              next(std::vector<DualPalindromicTreeNode*>(MaxLinks, nullptr)) {
            for (size_t id = 0; id < STRINGS; ++id) {
                num[id] = occAsMax[id] = occ[id] = 0;
            }
        }

        static const size_t STRINGS = 2;

        int length;
        size_t num[STRINGS];
        size_t occAsMax[STRINGS];
        size_t occ[STRINGS];
        bool id[STRINGS];
        DualPalindromicTreeNode* sufLink;
        std::vector<DualPalindromicTreeNode*> next;
    };

    // @NotTested
    template <class Node = DualPalindromicTreeNode<256>,
              int lo = 0, int hi = 256>
    class DualPalindromicTree {
    public:
        explicit DualPalindromicTree(const std::string &a, const std::string &b)
            : root1(new Node(-1)), root2(new Node(0)) {
            garbageCollector.push_back(root1);
            garbageCollector.push_back(root2);
            root1->sufLink = root1;
            root2->sufLink = root1;
            suffix = root2;
            push(a, 0);
            suffix = root2;
            push(b, 1);
            for (size_t id = 0; id < Node::STRINGS; ++id) {
                cntPalindromes[id] = 0;
            }
            calculateOccs();

        }

        ~DualPalindromicTree() {
            //TODO :: recursive deleting
            for(size_t i = 0; i < garbageCollector.size(); ++i) {
                delete garbageCollector[i];
            }
        }

        void findAddSuffix(Node* &cur, int pos, int &curLen,
                           const std::string &str) {
            while (true) {
                curLen = cur->length;
                if ((pos - 1 - curLen >= 0) &&
                        (str[pos - 1 - curLen] == str[pos])) {
                    break;
                }
                cur = cur->sufLink;
            }
        }

        void makeSuffixLink(Node* &cur, int pos, int &curLen, int letter,
                            const std::string &str) {
            while (true) {
                cur = cur->sufLink;
                curLen = cur->length;
                if ((pos - 1 - curLen >= 0) &&
                        (str[pos - 1 - curLen] == str[pos])) {
                    suffix->sufLink = cur->next[letter];
                    break;
                }
            }
        }

        void addLetter(int pos, const std::string &str, size_t id) {
            Node* cur = suffix;
            if (!((lo <= str[pos]) || (str[pos] < hi))) {
                std::string where = "sgtl::DualPalindromicTree::addLetter";
                std::string msg = " - bad character";
                throw new std::runtime_error(where + msg);
            }
            int letter = str[pos] - lo, curLen = 0;
            findAddSuffix(cur, pos, curLen, str);
            if (cur->next[letter] != nullptr) {
                suffix = cur->next[letter];
                ++suffix->occAsMax[id];
                return;
            }
            suffix = new Node(cur->length + 2);
            garbageCollector.push_back(suffix);
            ++suffix->occAsMax[id];


            cur->next[letter] = suffix;
            if (suffix->length == 1) {
                suffix->sufLink = root2;
                suffix->num[id] = 1;
                return;
            }
            makeSuffixLink(cur, pos, curLen, letter, str);
            suffix->num[id] = 1 + suffix->sufLink->num[id];
        }

        void push(const std::string &str, size_t id) {
            for (size_t i = 0; i < str.length(); ++i) {
                addLetter(i, str, id);
                for (size_t id = 0; id < Node::STRINGS; ++id) {
                    cntPalindromes[id] += suffix->num[id];
                }
            }
        }

        void calculateOccs() {
            for (int i = garbageCollector.size() - 1; i >= 0; --i) {
                Node *n = garbageCollector[i];
                for (size_t id = 0; id < Node::STRINGS; ++id) {
                    n->occ[id] = n->occAsMax[id];
                }
            }
            for (int i = garbageCollector.size() - 1; i >= 0; --i) {
                Node *n = garbageCollector[i];
                for (size_t id = 0; id < Node::STRINGS; ++id) {
                    n->sufLink->occ[id] += n->occ[id];
                }
            }

        }\

        /**  Calculates count (p,q,s,t) for two strings a, b
          where a[p..q] == b[s..t] and a[p..q] - palindrom. O(|A| + |B|) */
        size_t commonPalindromicSubstrings() {
            size_t ans = 0;
            for (int i = garbageCollector.size() - 1; i >= 2; --i) {
                Node *n = garbageCollector[i];
                ans += n->occ[0] * n->occ[1];
            }
            return ans;
        }

        Node* root1;
        Node* root2;
        Node* suffix;
        size_t cntPalindromes[2];

        std::vector<Node*> garbageCollector;
    };
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    size_t T;
    std::cin >> T;
    for (size_t t = 1; t <= T; ++t) {
        std::string a, b;
        std::cin >> a >> b;
        sgtl::DualPalindromicTree <sgtl::DualPalindromicTreeNode<'z' - 'a' + 1>,
                (size_t)'a', (size_t)('z' + 1)> pt(a, b);
        std::cout << "Case #" << t << ": "
                  << pt.commonPalindromicSubstrings() << "\n";
    }
    return 0;
}
