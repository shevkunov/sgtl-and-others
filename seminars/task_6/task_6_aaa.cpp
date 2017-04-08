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
    template <class Vector>
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

#define mp(a, b) std::make_pair(a, b)


void solution(const std::string& s, std::string& base, std::string& a) {
    std::pair<int, std::pair<int, int>> ans = mp(s.length(), mp(0, 0));
    std::vector<int> sm(s.length());
    std::vector<int> smt(s.length());
    std::vector<int> c(s.length());
    std::vector<int> ct(s.length());
    std::vector<int> nc(s.length());
    std::vector<int> cnt(33);

    for (size_t i = 0; i < s.length(); ++i) {
        sm[i] = i;
        c[i] = s[i] - 'a' + 1;
    }
    for (int i = 0; i < (int)sm.size(); ++i) {
        ++cnt[c[sm[i]]];
    }
    for (int i = 0; i + 1 < (int)cnt.size(); ++i) {
        cnt[i + 1] += cnt[i];
    }
    for (int i = smt.size() - 1; i >= 0; --i) {
        smt[--cnt[c[sm[i]]]] = sm[i];
    }
    smt.swap(sm);

    for (int l = 1; l < (int)s.length(); ++l) {
        cnt.assign(33, 0);
        for (int i = 0; i < (int)sm.size(); ++i) {
            nc[sm[i]] = (sm[i] + l < (int)s.length()) ?
                        (s[sm[i] + l] - 'a' + 1) : 0;
        }
        for (int i = 0; i < (int)sm.size(); ++i) {
            ++cnt[nc[sm[i]]];
        }
        for (int i = 0; i + 1 < (int)cnt.size(); ++i) {
            cnt[i + 1] += cnt[i];
        }
        for (int i = smt.size() - 1; i >= 0; --i) {
            smt[--cnt[nc[sm[i]]]] = sm[i];
        }
        smt.swap(sm);
        int classes = 0;

        ct[sm[0]] = 0;
        for (int i = 1; i < (int)sm.size(); ++i) {
            if ((nc[sm[i]] == nc[sm[i - 1]])
                    && (c[sm[i]] == c[sm[i - 1]])) {
                ct[sm[i]] = classes;
            } else {
                ct[sm[i]] = ++classes;
            }
        }
        ct.swap(c);

        int cls = 1;
        int right = sm[0] + l; // ..]
        for (int i = 1; i < (int)s.length(); ++i) {
            if (c[sm[i]] == c[sm[i - 1]]) {
                if (right < sm[i]) {
                    ++cls;
                    right = sm[i] + l;
                    ans = min(ans, mp((int)s.length() - (l + 1)*(cls - 1) + cls, mp(sm[i], sm[i] + l + 1)));
                }
            } else {
                cls = 1;
                right = sm[i] + l;
            }
        }

    }


    int ll = ans.second.second - ans.second.first;
    if (ll == 0) {
        base = "";
        a = s;
        //std::cout << "\n" << s << "\n";
        return;
    }
    sgtl::HashedString<> hs(s);
    sgtl::hash_t hash = hs.getRightedHash(ans.second.first, ans.second.second);
    base = s.substr(ans.second.first, ll);
    a = "";
    for (int i = 0; i < (int)s.length();) {
        if (hs.getRightedHash(i, i + ll) == hash) {
            a += 'A';
            i += ll;

        } else {
            a += s[i];
            ++i;
        }
    }

    //std::cout << base << "\n" << a << "\n";
    return;
}

void stress(const std::string& s, std::string& base, std::string& a) {
    sgtl::HashedString<> hs(s);
    typedef std::pair<sgtl::hash_t, std::pair<int, int>> myt;
    typedef std::pair<int, std::pair<int, int>> anst;

    anst ans = mp(s.length(), mp(0, 0));
    std::vector<int> yes;
    for (int i = 0; i < (int)s.length(); ++i) {
        yes.push_back(i);
    }
    for (int l = 2; l*2 <= (int)s.length(); ++l) {
        std::vector<myt> v;
        for (int i : yes) {
            v.push_back(mp(hs.getRightedHash(i, i + l), mp(i, i + l)));
        }
        yes.clear();
        std::sort(v.begin(), v.end());
        for (int j = 0; j < (int)v.size();) {
            int t = j + 1;
            int lst = j;
            int ready =1;
            while ((t < (int)v.size()) && (v[j].first == v[t].first)) {
                yes.push_back(v[t].second.first);
                if (v[lst].second.second <= v[t].second.first) {
                    ++ready;
                    lst = t;
                }
                ++t;
            }
            if (ready > 1) {
                yes.push_back(v[j].second.first);
            }
            anst n = mp(s.length() + (v[j].second.second - v[j].second.first)*(1-ready) + ready, v[j].second);
            if (ans > n) {
                ans = n;
            }
            j = t;
        }
    }

    int ll = ans.second.second - ans.second.first;
    if (ll == 0) {
        base = "";
        a = s;
        //std::cout << "\n" << s << "\n";
        return;
    }
    sgtl::hash_t hash = hs.getRightedHash(ans.second.first, ans.second.second);
    base = s.substr(ans.second.first, ll);
    a = "";
    for (int i = 0; i < (int)s.length();) {
        if (hs.getRightedHash(i, i + ll) == hash) {
            a += 'A';
            i += ll;

        } else {
            a += s[i];
            ++i;
        }
    }

    //std::cout << base << "\n" << a << "\n";
    return;
}

int main() {
    std::string s, base1, a1, base2, a2;
    std::cin >> s;
    solution(s, base1, a1);
    std::cout << base1 << "\n" << a1 << "\n";
    /*
    stress(s, base2, a2);
    std::cout << base2 << "\n" << a2 << "\n";
    if (base1.size() + a1.size() == base2.size() + a2.size()) {
        std::cout << "YES\n";
    } else {
        std::cout << "NO\n";
    }*/
    return 0;
}
