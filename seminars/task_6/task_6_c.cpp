#include <iostream>
#include <string>
#include <vector>
#include <ios>

namespace sgtl {
    typedef long long int hash_t;
    template <hash_t base_ = 256, hash_t module_ = 1000000007>
    class HashedString {
    public:
        HashedString(char* c) : s_(std::string(c)) {
            buildPows_();
            h_ = buildHash(s_);
        }

        HashedString(std::string& s) : s_(s) {
            buildPows_();
            h_ = buildHash(s_);
        }

        static std::vector<hash_t> buildHash(std::string& s) {
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
}

template <sgtl::hash_t base_, sgtl::hash_t module_>
std::vector<sgtl::hash_t> sgtl::HashedString<base_, module_>::pows_;

int main() {
    char s[100007];
    int n;
    scanf("%s", s);
    sgtl::HashedString<> hs = sgtl::HashedString<>(s);
    scanf("%d", &n);
    while (n--) {
        int l1, l2, r1, r2;
        scanf("%d %d %d %d", &l1, &r1, &l2, &r2);
        --l1; --l2;
        if (hs.compareSubStrings(l1, r1, hs, l2, r2)) {
            printf("Yes\n");
        } else {
            printf("No\n");
        }
    }
}
