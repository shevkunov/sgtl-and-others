#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <ios>
#include <algorithm>
#include <stdexcept>
#include <queue>


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
    /// Vector is something, which has [] and .size(), std::string for example
    template <class Vector>
    std::vector<size_t> buildLCPfromSufArr(const Vector& s, const std::vector<size_t>& sufArr) {
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

namespace sgtl {
    /// Vector is something, which has [] and .size(), std::string for example
    /** O(|v| log (|v|) */
    template <class Vector> // TODO : remove lo and hi from templates everywhere!
    std::vector<size_t> buildSuffixArray(const Vector &s,
                                         int lo = 0, int hi = 256) {
        for (size_t i = 0; i < s.size(); ++i) {
            if (!((lo <= (int)s[i]) && ((int)s[i] < hi))) {
                std::string where = "sgtl::buildSuffixArray";
                std::string what = " - bad character";
                throw std::runtime_error(where + what);
            }
        }

        std::vector<size_t> p(s.size());    // permutation
        std::vector<size_t> pn(s.size());   // new permutation

        std::vector<size_t> c(s.size());     // color
        std::vector<size_t> cn(s.size());    // new color

        std::vector<size_t> cnt(s.size());   // count

        // Zero phaze
        for (size_t i = 0; i < s.size(); ++i) {
            ++cnt[s[i] - lo];
        }
        for (size_t i = 0; i + 1 < cnt.size(); ++i) {
            cnt[i + 1] += cnt[i];
        }
        for (int i = (int)p.size() - 1; i >= 0; --i) {
            p[--cnt[s[i] - lo]] = i;
        }
        c[p[0]] = 0;
        size_t classes = 0;
        for (size_t i = 1; i < c.size(); ++i) {
            if (s[p[i]] != s[p[i - 1]]) {
                ++classes;
            }
            c[p[i]] = classes;
        }

        //Other phazes
        for (int len = 1; len < (int)s.size(); len *= 2) {
            for (size_t i = 0; i < p.size(); ++i) {
                if ((int)p[i] - len < 0) {
                    pn[i] = (int)(p[i] + s.size() - len);
                } else {
                    pn[i] = p[i] - len;
                }
            }

            cnt.assign(s.size(), 0);
            for (size_t i = 0; i < pn.size(); ++i) {
                ++cnt[c[pn[i]]];
            }
            for (size_t i = 0; i + 1 < cnt.size(); ++i) {
                cnt[i + 1] += cnt[i];
            }
            for (int i = (int)pn.size() - 1; i >= 0; --i) {
                p[--cnt[c[pn[i]]]] = pn[i];
            }

            int classes = 0;
            cn[p[0]] = 0;
            for (size_t i = 1; i < p.size(); ++i) {
                if ((c[p[i]] != c[p[i - 1]]) ||
                        (c[(p[i] + len) % s.size()] !=
                         c[(p[i - 1] + len) % s.size()])) {
                    ++classes;
                }
                cn[p[i]] = classes;
            }
            c.swap(cn);
        }
        return p;
    }
}

namespace sgtl {
    void findRefren(std::vector<size_t> &s, std::pair<long long int, size_t> &ans,
               std::vector<size_t> &sufMas, std::vector<size_t> &lcp) {
        size_t m = 0;
        for (size_t i = 0; i < s.size(); ++i) {
            if (m < s[i]) {
                m = s[i];
            }
        }
        s.push_back(0);
        sufMas = buildSuffixArray(s, 0, m + 1);
        lcp = buildLCPfromSufArr(s, sufMas);

        ans = std::make_pair(-1, 0);
        std::vector<size_t> h;
        lcp.push_back(0);
        for (size_t i = 0; i < lcp.size(); ++i) {
            while (!h.empty() && (lcp[h.back()] > lcp[i])) {
                size_t hGet = h.back();
                h.pop_back();
                long long int s = (long long int)(i - h.back()) * lcp[hGet];
                ans = max(ans, std::make_pair(s, hGet));
            }
            h.push_back(i);
        }
        s.pop_back();
    }
}


namespace sgtl {
    class AhoCorasik {
    public:
        struct Vertex {
            Vertex(size_t size)
                : Vertex(size, -1, -1) {
            }
            Vertex(size_t size, int parent, int parentChar)
               : leaf(0), linkLeafed(0), link(-1),
                 parent(parent), parentChar(parentChar),
                 next(std::vector<int>(size, -1)),
                 go(std::vector<int>(size, -1)) {
            }
            long long leaf;
            long long linkLeafed;
            int link;
            int parent;
            int parentChar;
            std::vector<int> next;
            std::vector<int> go;
        };

        AhoCorasik(int lo = 0, int hi = 256)
            : lo(lo), hi(hi), t(std::vector<Vertex>(1, Vertex(hi - lo))) {
        }

        AhoCorasik(const std::string &s, int lo = 0, int hi = 256)
            : lo(lo), hi(hi), t(std::vector<Vertex>(1, Vertex(hi - lo))) {
            addString(s);
        }

        int nextOrCreate(int vertex, int letter) {
            if (t[vertex].next[letter] == -1) {
                t.push_back(Vertex(hi - lo, vertex, letter));
                t[vertex].next[letter] = t.size() - 1;
            }
            return t[vertex].next[letter];
        }

        void addString(const std::string &s) {
            int vertex = 0;
            for (size_t i = 0; i < s.length(); ++i) {
                if (!((lo <= s[i]) && (s[i] < hi))) {
                    std::string where = "sgtl::AxoCorasic::addString";
                    std::string what = " - bad character";
                    throw std::runtime_error(where + what);
                }
                int letter = s[i] - lo;
                vertex = nextOrCreate(vertex, letter);
            }
            ++t[vertex].leaf;
        }

        int getLink(int vertex) {
            if (t[vertex].link == -1) {
                if ((vertex == 0) || (t[vertex].parent == 0)) {
                    t[vertex].link = 0;
                } else {
                    t[vertex].link = go(getLink(t[vertex].parent), t[vertex].parentChar);
                }
            }
            return t[vertex].link;
        }

        int go(int vertex, int letter) {
            if (t[vertex].go[letter] == -1) {
                if (t[vertex].next[letter] != -1) {
                    t[vertex].go[letter] = t[vertex].next[letter];
                } else {
                    if (vertex == 0) {
                        t[vertex].go[letter] = 0;
                    } else {
                        t[vertex].go[letter] = go(getLink(vertex), letter);
                    }
                }
            }
            return t[vertex].go[letter];
        }

        void linkLeafInitialize() {
            std::queue<int> order;
            order.push(0);
            while (!order.empty()) {
                int v = order.front();
                order.pop();
                getLink(v);
                t[v].linkLeafed = t[t[v].link].linkLeafed + t[v].leaf;

                for (int e = 0; e < hi - lo; ++e) {
                    if (t[v].next[e] != -1)
                    order.push(t[v].next[e]);
                }
            }
        }

        int lo, hi;
        std::vector<Vertex> t;
    };
}

namespace sgtl {
    /** Searches maximum k, where S = A + (T + ... + T) + B = A + T^k + B
     * ans T is not empty. Returns <T.size(), start>*/
    class MaximumMultipleEnteringsSearch {
    public:
        static std::pair<size_t, size_t> search(const std::string &s) {
            if (s.size() == 0) {
                return std::make_pair(0, 0);
            } else {
                return search(s, 0, s.size());
            }
        }

        static std::pair<size_t, size_t> stupidSolutions(const std::string &s) {
            HashedString<> hs(s);
            std::pair<size_t, size_t> ans = {0, 0};
            for (size_t start = 0; start < s.size(); ++start) {
                for (size_t l = 1; l < s.size() - start; ++l) {
                    size_t cnt = 1;
                    size_t i = 0;
                    while (hs.compareSubStrings(start + i*l,
                           start + (i + 1) * l, hs, start + (i + 1) * l,
                           start + (i + 2) * l)) {
                        ++i;
                    }
                    cnt += i;
                    ans = std::max(ans, {cnt, start});
                }
            }
            return ans;
        }
        static std::string randString(size_t size,
                                      char lo = 'a', char hi = 'z' + 1) {
            std::string s;
            while (size--) {
                s += (char)(lo + rand() % (hi - lo));
            }
            return s;
        }

    private:
        static std::pair<size_t, size_t> search(const std::string &s,
                                                size_t begin, size_t end,
                                                char newSymbol = '$') {
            if (end - begin == 1) {
                return std::make_pair(1, 0);
            } else {
                size_t center = (begin + end) / 2;
                std::string s1 = s.substr(begin, center - begin);
                /// TODO : we can not to build this strings
                std::string s2 = s.substr(center, end - center);
                std::string rs; // = rev(s1) + '$' + rev(s2);
                std::string fs = s2 + newSymbol + s1;
                for (int i = (int)s1.size() - 1; i >= 0; --i) {
                    rs.push_back(s1[i]);
                }
                rs.push_back(newSymbol);
                size_t rS2PositionInRs = rs.size();
                for (int i = (int)s2.size() - 1; i >= 0; --i) {
                    rs.push_back(s2[i]);
                }
                std::vector<size_t> z2 = zFunction(s2);
                std::vector<size_t> zrs = zFunction(rs);
                std::vector<size_t> zfs = zFunction(fs);

                std::pair<size_t, size_t> ans =
                        std::max(search(s, begin, center),
                                 search(s, center, end));

                for (size_t l = 1; l < s2.size(); ++l) {
                    ///Phase
                    if (l == 1) {
                        size_t countS2 = z2[1] + 1;
                        size_t countS1 = (zrs[1] + 1) * (s1.back() == s2.front());
                        ans = std::max(ans, {countS1 + countS2, l});
                    } else {
                        size_t zl = z2[l];
                        if (zl >= l) {
                            size_t countS2 = (z2[l] / l + 1);
                            size_t countS1 = 0;
                            if (zl % l == 0) {
                                if (zrs[zrs.size() - l] >= l) {
                                    countS1 += (zrs[l] / l) + 1;
                                }
                            } else {
                                if (zrs[zrs.size() - l] >= zl % l) {
                                    countS1 += (zrs[l] / l) + 1;
                                }
                            }

                            ans = max(ans, {countS1 + countS2, center});
                        } else {
                            size_t countS2 = 1;
                            size_t countS1 = 0;
                            if ((zrs[zrs.size() - l] >= l - zl) &&
                                (zl)){
                                /// can build, but only one more
                                ++countS2;
                                size_t zrsl = zrs[l];
                                if (zrs[l] >= l - zl) {
                                    if ((zrsl >= zl) && (zfs[zfs.size() - l] >= zl)) {
                                        countS1 += zrs[l] / l + 1;
                                    } else {
                                        if (zfs[zfs.size() - l] >= zl){
                                            ++countS1;
                                        }
                                    }
                                }
                            } else {
                                size_t zrsl = zrs[l];
                                if ((zrsl >= l) && (zfs[zfs.size() - l] >= l - zl)) {
                                    countS1 += zrs[l] / l;
                                    if (zl == 0) {
                                        ++countS1;
                                    }
                                } else {
                                    if (zfs[zfs.size() - l] >= l - zrsl){
                                        ++countS1;
                                    }
                                }

                            }
                            ans = max(ans, {countS1 + countS2, center - l + zl});
                        }
                    }

                }
                if (zrs[rS2PositionInRs] == end - center) {
                    ans = max(ans, {2, center});
                }
                return ans;
            }
        }
    };
}



int main() {
    //ababab - 3
    //caddaccaddac - 2
    /*
    std::ios_base::sync_with_stdio(false);
    std::string s;
    std::cin >> s;
    std::cout << sgtl::MaximumMultipleEnteringsSearch::search(s).first;*/
    int phase = 0;
    int phasesGone = 0;
    std::cin >> phasesGone;
    while (true) {
        std::string s = sgtl::MaximumMultipleEnteringsSearch::randString(10, 'a', 'h');
        std::cout << "RUN <" << ++phase << ">\n";
        if (phase >= phasesGone) {
            std::cout << "STR = " << s << "\n";
            std::cout << sgtl::MaximumMultipleEnteringsSearch::search(s).first;
            if (sgtl::MaximumMultipleEnteringsSearch::search(s).first !=
                    sgtl::MaximumMultipleEnteringsSearch::stupidSolutions(s).first) {
                std::cout << "FAIL\n";
                return 0;
            } else {
                std::cout << "OK\n";
            }
        }
    }
    return 0;
}
