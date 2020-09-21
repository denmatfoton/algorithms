/**
 * @file big_int.h
 * @brief Big integer data structure.
 */
#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <type_traits>
#if defined(_MSC_VER)
     /* Microsoft C/C++-compatible compiler */
     #include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
     /* GCC-compatible compiler, targeting x86/x86-64 */
     #include <x86intrin.h>
#elif defined(__GNUC__) && defined(__ARM_NEON__)
     /* GCC-compatible compiler, targeting ARM with NEON */
     #include <arm_neon.h>
#endif


#if 1
template <typename T = int>
class BigInt {
    void Trim() {
        while (val.size() && val.back() == 0) val.pop_back();
        if (val.empty()) {
            sig = 0;
            val.push_back(0);
        }
    }

    static std::vector<T> Sub(const std::vector<T>& a, const std::vector<T>& b) {
        T carry = 0;
        auto al = a.size();
        std::vector<T> ans(al);
        for (size_t i = 0; i < al; ++i) {
            carry += a[i] - (i < b.size() ? b[i] : 0);
            if (carry < 0) {
                ans[i] = carry + base;
                carry = -1;
            }
            else {
                ans[i] = carry;
                carry = 0;
            }
        }
        while (ans.size() > 1 && ans.back() == 0) ans.pop_back();
        return ans;
    }

    static std::vector<T> Add(const T* a, const T* b, size_t al, size_t bl, size_t as = 0) {
        T carry = 0;
        auto n = std::max(al, bl);
        std::vector<T> ans(as);
        if (as) memcpy(ans.data(), b, sizeof(T) * as);
        for (size_t i = 0; i < n; ++i) {
            if (i < al) carry += a[i];
            if (i + as < bl) carry += b[i + as];
            ans.push_back(carry % base);
            carry /= base;
        }
        if (carry) ans.push_back(carry);
        return ans;
    }

    static std::vector<T> Add(const std::vector<T>& a, const std::vector<T>& b, size_t as = 0) {
        return Add(a.data(), b.data(), a.size(), b.size(), as);
    }

    static std::vector<T> Multiply(const std::vector<T>& a, const std::vector<T>& b) {
        return Multiply(a.data(), b.data(), a.size(), b.size());
    }

    static std::vector<T> Multiply(const T* a, const T* b, size_t al, size_t bl) {
        for (; al; al--) if (a[al - 1]) break;
        for (; bl; bl--) if (b[bl - 1]) break;
        if (al < bl) {
            std::swap(a, b);
            std::swap(al, bl);
        }

        if (bl == 0) return { 0 };
        if (bl <= 2) {
            int64_t carry = 0;
            std::vector<T> res0(al);
            for (size_t i = 0; i < al; ++i, carry /= base) {
                carry += int64_t(a[i]) * b[0];
                res0[i] = carry % base;
            }
            if (carry) res0.push_back(T(carry));
            if (bl == 1) return res0;
            std::vector<T> res1(al);
            carry = 0;
            for (size_t i = 0; i < al; ++i, carry /= base) {
                carry += int64_t(a[i]) * b[1];
                res1[i] = carry % base;
            }
            if (carry) res1.push_back(T(carry));
            return Add(res1, res0, 1);
        }

        T l = T(al / 2);
        T ha = T(al) - l;
        T hb = T(bl) - l;
        if (hb <= 0) {
            auto ah_bl = Multiply(a + l, b, ha, bl);
            auto al_bl = Multiply(a, b, l, bl);
            return Add(ah_bl, al_bl, l);
        }

        auto ah_x_bh = Multiply(a + l, b + l, ha, hb);
        auto al_x_bl = Multiply(a, b, l, l);
        auto ah_x_bh_P_al_x_bl = Add(ah_x_bh, al_x_bl);

        auto ah_p_al = Add(a + l, a, ha, l, 0);
        auto bh_p_bl = Add(b + l, b, hb, l, 0);
        auto ah_p_al_X_bh_p_bl = Multiply(ah_p_al, bh_p_bl);

        auto temp = Sub(ah_p_al_X_bh_p_bl, ah_x_bh_P_al_x_bl);
        temp = Add(temp, al_x_bl, l);
        return Add(ah_x_bh, temp, l * 2);
    }

    std::vector<T> val;
    int sig = 0;
    static constexpr T base = T(1e9);
public:
    BigInt() : val(1) { }

    BigInt(const char* s) {
        if (s[0] == '-') {
            sig = 1;
            s++;
        }
        size_t n = strlen(s);
        if (n == 0) {
            sig = 0;
            val.push_back(0);
            return;
        }
        size_t i = n % 9;
        T v = 0;
        for (size_t j = 0; j < i; j++)
            v = v * 10 + (s[j] - '0');
        if (v) val.push_back(v);
        for (; i < n; i += 9) {
            v = 0;
            for (auto j = i; j < i + 9; j++)
                v = v * 10 + (s[j] - '0');
            val.push_back(v);
        }
        reverse(val.begin(), val.end());
    }

    BigInt(const std::string& s) : BigInt(s.c_str()) {}

    BigInt(int64_t x) {
        if (x < 0) {
            sig = 1;
            x = -x;
        }
        for (; x; x /= base)
            val.push_back(x % base);
        reverse(val.begin(), val.end());
    }

    std::string ToString() const {
        std::string out = (sig ? "-" : "") + std::to_string(val.back());
        auto i = out.size();
        out.resize(i + (val.size() - 1) * 9);
        char* s = &out[0] + i;
        for (i = val.size() - 1; i--; s += 9)
            snprintf(s, 10, "%09d", val[i]);
        return out;
    }

    template <typename IntT>
    IntT ToInt() const {
        IntT res = 0;
        for (auto v : val) {
            res *= base;
            res += v;
        }
        if constexpr (std::is_signed(res)) {
            if (sig) res = -res;
        }
        return res;
    }

    friend std::ostream& operator << (std::ostream& out, const BigInt& a) {
        return out << a.ToString().c_str();
    }


    bool operator >= (const BigInt& b) const {
        if (sig != b.sig) return sig < b.sig;
        if (val.size() != b.val.size())
            return (val.size() > b.val.size()) ^ sig;
        for (size_t i = val.size(); i--;)
            if (val[i] != b.val[i])
                return (val[i] > b.val[i]) ^ sig;
        return true;
    }

    bool operator > (const BigInt& b) const {
        if (sig != b.sig) return sig < b.sig;
        if (val.size() != b.val.size())
            return (val.size() > b.val.size()) ^ sig;
        for (size_t i = val.size(); i--;)
            if (val[i] != b.val[i])
                return (val[i] > b.val[i]) ^ sig;
        return false;
    }

    bool operator < (const BigInt& b) const {
        return b > * this;
    }

    bool operator <= (const BigInt& b) const {
        return b >= *this;
    }

    bool operator == (const BigInt& b) const {
        if (sig != b.sig) return false;
        if (val.size() != b.val.size()) return false;
        for (size_t i = val.size(); i--;)
            if (val[i] != b.val[i]) return false;
        return true;
    }


    BigInt operator + (const BigInt& b) const {
        if (sig != b.sig) {
            auto temp = b;
            temp.sig ^= 1;
            return *this - temp;
        }
        BigInt ans;
        ans.sig = sig;
        ans.val = Add(val, b.val, 0);
        return ans;
    }

    BigInt operator + (long b) {
        return *this + BigInt(b);
    }

    BigInt& operator ++ () {
        return *this = *this + 1;
    }

    BigInt operator ++ (int) {
        auto old = *this;
        *this = *this + 1;
        return old;
    }

    BigInt& operator += (const BigInt& b) {
        return *this = *this + b;
    }

    BigInt& operator += (long b) {
        return *this = *this + b;
    }


    BigInt operator - (const BigInt& b) const {
        if (sig != b.sig) {
            auto temp = b;
            temp.sig ^= 1;
            return *this + temp;
        }
        if ((sig == 0 && *this < b) || (sig == 1 && *this > b)) {
            auto ans = b - *this;
            ans.sig ^= 1;
            return ans;
        }
        BigInt ans;
        ans.sig = sig;
        ans.val = Sub(val, b.val);
        ans.Trim();
        return ans;
    }

    BigInt operator - (long b) {
        return *this - BigInt(b);
    }

    BigInt& operator -- () {
        return *this = *this - 1;
    }

    BigInt operator -- (int) {
        auto old = *this;
        *this = *this - 1;
        return old;
    }

    BigInt& operator -= (const BigInt& b) {
        return *this = *this - b;
    }

    BigInt& operator -= (long b) {
        return *this = *this - b;
    }


    size_t DigCount() const {
        size_t cnt = (val.size() - 1) * 9;
        for (auto t = val.back(); t; t /= 10, ++cnt);
        return cnt;
    }

    size_t DigSum() const {
        size_t sum = 0;
        for (auto v : val) {
            for (; v; v /= 10) sum += v % 10;
        }
        return sum;
    }

    size_t DigSum(int num) const {
        size_t sum = 0;
        size_t i = val.size() - 1;
        for (auto v = val[i]; v; v /= 10, --num) sum += v % 10;
        if (num < 0) {
            for (auto v = val[i]; num++; v /= 10) sum -= v % 10;
            return sum;
        }
        while (i-- && num > 0) {
            for (auto v = val[i]; v; v /= 10) sum += v % 10;
            num -= 9;
            if (num < 0) {
                for (auto v = val[i]; num++; v /= 10) sum -= v % 10;
                break;
            }
        }
        return sum;
    }

    void Reverse() {
        reverse(val.begin(), val.end());
        int r = 0, d = 1;
        for (int t = val[0]; t; t /= 10) {
            r *= 10;
            d *= 10;
            r += t % 10;
        }
        val[0] = r;
        int rd = base / d;
        for (int i = 1; i < val.size(); ++i) {
            r = 0;
            for (int t = val[i], f = 0; ++f < 10; t /= 10) {
                r *= 10;
                r += t % 10;
            }
            val[i] = r / rd;
            val[i - 1] += (r % rd) * d;
        }
    }

    void Swap(BigInt& b) {
        swap(val, b.val);
        swap(sig, b.sig);
    }


    BigInt operator * (const BigInt& b) const {
        BigInt ans;
        ans.sig = sig ^ b.sig;
        ans.val = Multiply(val, b.val);
        ans.Trim();
        return ans;
    }

    BigInt operator * (long b) {
        return *this * BigInt(b);
    }

    BigInt& operator *= (const BigInt& b) {
        return *this = *this * b;
    }
};
#else
template <typename T = int>
class BigInt {
    void Trim() {
        while (val.size() && val.back() == 0) val.pop_back();
        if (val.empty()) {
            sig = 0;
            val.push_back(0);
        }
    }

    static std::vector<T> Sub(const std::vector<T>& a, const std::vector<T>& b) {
        T carry = 0;
        auto al = a.size();
        std::vector<T> ans(al);
        for (size_t i = 0; i < al; ++i) {
            carry += a[i] - (i < b.size() ? b[i] : 0);
            if (carry < 0) {
                ans[i] = carry + base;
                carry = -1;
            }
            else {
                ans[i] = carry;
                carry = 0;
            }
        }
        while (ans.size() > 1 && ans.back() == 0) ans.pop_back();
        return ans;
    }

    static void InplaceSub(std::vector<T>& a, const std::vector<T>& b, bool rev = false) {
        T carry = 0;
        if (rev) {
            auto bl = b.size();
            a.resize(bl);
            for (size_t i = 0; i < bl; ++i) {
                carry += b[i] - a[i];
                if (carry < 0) {
                    a[i] = carry + base;
                    carry = -1;
                }
                else {
                    a[i] = carry;
                    carry = 0;
                }
            }
        }
        else {
            auto al = a.size();
            for (size_t i = 0; i < al; ++i) {
                carry += a[i] - (i < b.size() ? b[i] : 0);
                if (carry < 0) {
                    a[i] = carry + base;
                    carry = -1;
                }
                else {
                    a[i] = carry;
                    carry = 0;
                }
            }
        }
        while (a.size() > 1 && a.back() == 0) a.pop_back();
    }

    /**
     * @param a Pointer to the first integer data
     * @param b Pointer to the second integer data
     * @param al Length of the first integer data
     * @param bl Length of the second integer data
     * @param as Shift a to the left before add operation.
     */
    static std::vector<T> Add(const T* a, const T* b, size_t al, size_t bl, size_t as = 0) {
        T carry = 0;
        auto n = std::max(al, bl);
        std::vector<T> ans(as);
        if (as) memcpy(ans.data(), b, sizeof(T) * as);
        for (size_t i = 0; i < n; ++i) {
            if (i < al) carry += a[i];
            if (i + as < bl) carry += b[i + as];
            ans.push_back(carry % base);
            carry /= base;
        }
        if (carry) ans.push_back(carry);
        return ans;
    }

    static void InplaceAdd(std::vector<T>& a, const std::vector<T>& b, size_t as = 0) {
        T carry = 0;
        auto al = a.size(), bl = b.size();
        auto n = std::max(al + as, bl);
        if (as) {
            a.resize(al + as);
            memmove(a.data() + as, a.data(), sizeof(T) * al);
            memcpy(a.data(), b.data(), sizeof(T) * as);
        }
        for (size_t i = as; i < n; ++i, carry /= base) {
            if (i >= a.size()) a.push_back(0);
            carry += a[i];
            if (i < bl) carry += b[i];
            a[i] = carry % base;
        }
        if (carry) a.push_back(carry);
    }

    static int Compare(const std::vector<T>& a, const std::vector<T>& b) {
        if (a.size() != b.size())
            return a.size() > b.size() ? 1 : -1;
        for (size_t i = a.size(); i--;) {
            if (a[i] != b[i])
                return a[i] > b[i] ? 1 : -1;
        }
        return 0;
    }

    static std::vector<T> Add(const std::vector<T>& a, const std::vector<T>& b, size_t as = 0) {
        return Add(a.data(), b.data(), a.size(), b.size(), as);
    }

    static std::vector<T> Multiply(const std::vector<T>& a, const std::vector<T>& b) {
        return Multiply(a.data(), b.data(), a.size(), b.size());
    }

    static std::vector<T> Multiply(const T* a, const T* b, size_t al, size_t bl) {
        for (; al; al--) if (a[al - 1]) break;
        for (; bl; bl--) if (b[bl - 1]) break;
        if (al < bl) {
            std::swap(a, b);
            std::swap(al, bl);
        }

        if (bl == 0) return { 0 };
        if (bl <= 2) {
            int64_t carry = 0;
            std::vector<T> res0(al);
            for (size_t i = 0; i < al; ++i, carry /= base) {
                carry += int64_t(a[i]) * b[0];
                res0[i] = carry % base;
            }
            if (carry) res0.push_back(T(carry));
            if (bl == 1) return res0;
            std::vector<T> res1(al);
            carry = 0;
            for (size_t i = 0; i < al; ++i, carry /= base) {
                carry += int64_t(a[i]) * b[1];
                res1[i] = carry % base;
            }
            if (carry) res1.push_back(T(carry));
            InplaceAdd(res1, res0, 1);
            return res1;
        }

        T l = T(al / 2);
        T ha = T(al) - l;
        T hb = T(bl) - l;
        if (hb <= 0) {
            auto ah_bl = Multiply(a + l, b, ha, bl);
            auto al_bl = Multiply(a, b, l, bl);
            InplaceAdd(ah_bl, al_bl, l);
            return ah_bl;
        }

        auto ah_x_bh = Multiply(a + l, b + l, ha, hb);
        auto al_x_bl = Multiply(a, b, l, l);
        auto ah_x_bh_P_al_x_bl = Add(ah_x_bh, al_x_bl);

        auto ah_p_al = Add(a + l, a, ha, l, 0);
        auto bh_p_bl = Add(b + l, b, hb, l, 0);
        auto temp = Multiply(ah_p_al, bh_p_bl);

        InplaceSub(temp, ah_x_bh_P_al_x_bl);
        InplaceAdd(temp, al_x_bl, l);
        InplaceAdd(ah_x_bh, temp, l * 2);
        return ah_x_bh;
    }


    static void Shift(std::vector<T>& v, int s) {
        if ((v.size() == 1 && v.front() == 0) || s == 0) return;
        auto n = abs(s) / base_bits_num;
        auto r = abs(s) % base_bits_num;
        if (s > 0) {
            if (r) {
                T carry = 0;
                for (size_t i = 0; i < v.size(); ++i) {
                    T cur = ((v[i] << r) & (base - 1)) | carry;
                    carry = v[i] >> (base_bits_num - r);
                    v[i] = cur;
                }
                if (carry) v.push_back(carry);
            }
            if (n) {
                v.insert(v.begin(), n, 0);
            }
        }
        else {
            if (v.size() <= n) {
                v.clear();
                v.push_back(0);
                return;
            }
            v.erase(v.begin(), v.begin() + n);
            if (r) {
                T carry = 0;
                for (size_t i = v.size(); i--;) {
                    T cur = (v[i] >> r) | carry;
                    carry = (v[i] << (base_bits_num - r)) & (base - 1);
                    v[i] = cur;
                }
            }
        }
    }


    static size_t BitsNum(const std::vector<T>& x) {
        if (x.back() == 0) return 0;
        int d_bits_num = T_bits_num - __builtin_clz(x.back());
        return base_bits_num * (x.size() - 1) + d_bits_num;
    }


#if 0
    struct Reciprocal {
        std::vector<T> val;
        T e;
        int sig;
    };

    static Reciprocal RecipMul(const Reciprocal& a, const Reciprocal& b, size_t t_len) {
        auto a_len = BitsNum(a.val), b_len = BitsNum(b.val);
        Reciprocal res;
        res.val = Multiply(a.val, b.val);
        auto prod_len = BitsNum(res.val);
        // normalize
        int s = (base_bits_num - ((prod_len - 1) % base_bits_num) % base_bits_num;
        Shift(res.val, s));
        if (res.val.size() > t_len + 1) {
            res.val.erase(res.val.begin(), res.val.end() - t_len - 1);
        }
        res.e = a.e + b.e + prod_len - a_len - b_len + 1;
        return res;
    }


    static void RecipAdd(Reciprocal& a, const Reciprocal& b, size_t t_len) {
        int e_dif = a.e - b.e;
        auto bv = b.val;
        t_len *= base_bits_num;
        if (e_dif >= 0) {
            if (e_dif > t_len) return;
            int bl = BitsNum(bv), al = BitsNum(a.val);
            int al_new = min(int(t_len) + 1, max(al, bl + e_dif));
            Shift(a.val, al_new - al);
            Shift(bv, al_new - e_dif - bl);
            InplaceAdd(a.val, bv);
            al = BitsNum(a.val);
            Shift(a.val, al_new - al);
            a.e += al - al_new;
        }
        else {
            if (e_dif < -t_len) {
                a = b;
                return;
            }

        }
    }


    static void RecipSub(Reciprocal& a, const Reciprocal& b, size_t t_len) {
        int e_dif = a.e - b.e;
        auto bv = b.val;
        t_len *= base_bits_num;
        if (e_dif >= 0) {
            if (e_dif > t_len) return;
            int bl = BitsNum(bv), al = BitsNum(a.val);
            int al_new = min(int(t_len) + 1, max(al, bl + e_dif));
            Shift(a.val, al_new - al);
            Shift(bv, al_new - e_dif - bl);
            InplaceSub(a.val, bv);
            al = BitsNum(a.val);
            Shift(a.val, al_new - al);
            a.e += al - al_new;
        }
        else {
            if (e_dif < -t_len) {
                a = b;
                return;
            }

        }
    }


    static Reciprocal CalcReciprocal(const std::vector<T>& d, size_t t_len) {
        vector<T> rec48_17(t_len + 1), rec32_17(t_len + 1);
        size_t i = t_len;
        rec48_17[i] = 1;
        rec32_17[i] = 1;
        uint32_t x48 = 0x5A5A5A5AU, x32 = 0xE1E1E1E1U;
        while (i--) {
            rec48_17[i] = x48 >> (T_bits_num - base_bits_num);
            x48 = (x48 >> (T_bits_num - base_bits_num)) || (x48 << base_bits_num);
            rec32_17[i] = x32 >> (T_bits_num - base_bits_num);
            x32 = (x32 >> (T_bits_num - base_bits_num)) || (x32 << base_bits_num);
        }
        int d_bits_num = T_bits_num - __builtin_clz(d.back()) - 1;
        /*int s = (base_bits_num - d_bits_num) % base_bits_num;
        auto D = d;
        if (s) Shift(D, s);*/
        int d_len = BitsNum(d), x32_len = t_len * base_bits_num;
        auto x32_d = Multiply(x32, d);
        int x32_d_len = BitsNum(x32_d);


    }


    static std::vector<T> Divide(const std::vector<T>& n, const std::vector<T>& n) {
        
        
    }
#endif

    std::vector<T> val;
    int sig = 0;
    static constexpr size_t base_bits_num = 30;
    static constexpr size_t T_bits_num = sizeof(T) * 8;
    static constexpr T base = T(1 << base_bits_num);
public:
    BigInt() : val(1) {}

    BigInt(const char* s) {
        if (s[0] == '-') {
            sig = 1;
            s++;
        }
        size_t n = strlen(s);
        if (n == 0) {
            sig = 0;
            val.push_back(0);
            return;
        }
        size_t i = n % 9;
        T v = 0;
        for (size_t j = 0; j < i; j++)
            v = v * 10 + (s[j] - '0');
        val.push_back(v);
        for (; i < n; i += 9) {
            v = 0;
            for (auto j = i; j < i + 9; j++)
                v = v * 10 + (s[j] - '0');

            int64_t carry = 0;
            for (size_t j = 0; j < val.size(); ++j, carry /= base) {
                carry += int64_t(val[j]) * 1'000'000'000;
                val[j] = carry % base;
            }
            if (carry) val.push_back(T(carry));

            val[0] += v;
        }
        int64_t carry = 0;
        for (size_t j = 0; j < val.size(); ++j, carry /= base) {
            carry += val[j];
            val[j] = carry % base;
        }
        if (carry) val.push_back(T(carry));
    }

    BigInt(const std::string& s) : BigInt(s.c_str()) {}

    BigInt(int64_t x) {
        if (x < 0) {
            sig = 1;
            x = -x;
        }
        for (; x; x /= base)
            val.push_back(x % base);
        reverse(val.begin(), val.end());
    }

    std::string ToString(int radix = 10) const {
        std::string out;

        int64_t mod = 1;
        for (; mod < base; mod *= radix);
        mod /= radix;

        auto temp = val;
        while (temp.size()) {
            int64_t rem = 0;
            if (temp.back() < mod) {
                rem = temp.back();
                temp.pop_back();
            }
            for (auto i = temp.size(); i--;) {
                int64_t cur = temp[i] + rem * base;
                temp[i] = T(cur / mod);
                rem = cur % mod;
            }
            for (int i = 0; i < 9; ++i, rem /= radix) {
                char c = '0' + char(rem % radix);
                if (c > '9') c += 'A' - '0' - 10;
                out += c;
            }
        }

        while (out.size() && out.back() == '0') out.pop_back();
        if (sig) out += '-';

        reverse(out.begin(), out.end());
        return out;
    }

    friend std::ostream& operator << (std::ostream& out, const BigInt& a) {
        return out << a.ToString().c_str();
    }


    bool operator >= (const BigInt& b) const {
        if (sig != b.sig) return sig < b.sig;
        int c = Compare(val, b.val);
        return c ? (c > 0) ^ sig : true;
    }

    bool operator > (const BigInt& b) const {
        if (sig != b.sig) return sig < b.sig;
        int c = Compare(val, b.val);
        return c ? (c > 0) ^ sig : false;
    }

    bool operator < (const BigInt& b) const {
        return b > * this;
    }

    bool operator <= (const BigInt& b) const {
        return b >= *this;
    }

    bool operator == (const BigInt& b) const {
        if (sig != b.sig) return false;
        if (val.size() != b.val.size()) return false;
        for (size_t i = val.size(); i--;)
            if (val[i] != b.val[i]) return false;
        return true;
    }


    BigInt operator + (const BigInt& b) const {
        auto ans = *this;
        ans += b;
        return ans;
    }

    BigInt operator + (long b) {
        return *this + BigInt(b);
    }

    BigInt& operator ++ () {
        return *this += 1;
    }

    BigInt operator ++ (int) {
        auto old = *this;
        *this += 1;
        return old;
    }

    BigInt& operator += (const BigInt& b) {
        if (sig != b.sig) {
            int c = Compare(val, b.val);
            if (c) {
                InplaceSub(val, b.val, c < 0);
                sig ^= c < 0;
            }
            else {
                val = { 0 };
                sig = 0;
            }
            return *this;
        }
        InplaceAdd(val, b.val);
        return *this;
    }

    BigInt& operator += (long b) {
        return *this += BigInt(b);
    }


    BigInt operator - (const BigInt& b) const {
        auto ans = *this;
        ans -= b;
        return ans;
    }

    BigInt operator - (long b) {
        return *this - BigInt(b);
    }

    BigInt& operator -- () {
        return *this -= 1;
    }

    BigInt operator -- (int) {
        auto old = *this;
        *this =- 1;
        return old;
    }

    BigInt& operator -= (const BigInt& b) {
        if (sig != b.sig) {
            InplaceAdd(val, b.val);
            return *this;
        }
        int c = Compare(val, b.val);
        if (c) {
            InplaceSub(val, b.val, c < 0);
            sig ^= c < 0;
        }
        else {
            Clear();
        }
        return *this;
    }

    BigInt& operator -= (long b) {
        return *this -= BigInt(b);
    }


    BigInt operator <<= (size_t s) {
        Shift(val, s);
    }

    BigInt operator << (size_t s) {
        auto ans = *this;
        return ans <<= s;
    }

    BigInt operator >>= (size_t s) {
        Shift(val, -int(s));
    }

    BigInt operator >> (size_t s) {
        auto ans = *this;
        return ans >>= s;
    }


    void Clear() {
        val = { 0 };
        sig = 0;
    }

    size_t DigCount() {
        size_t cnt = (val.size() - 1) * 9;
        for (auto t = val.back(); t; t /= 10, ++cnt);
        return cnt;
    }

    size_t DigSum() {
        size_t sum = 0;
        for (auto v : val) {
            for (; v; v /= 10) sum += v % 10;
        }
        return sum;
    }


    BigInt operator * (const BigInt& b) const {
        BigInt ans;
        ans.sig = sig ^ b.sig;
        ans.val = Multiply(val, b.val);
        ans.Trim();
        return ans;
    }

    BigInt operator * (long b) {
        return *this * BigInt(b);
    }

    BigInt& operator *= (const BigInt& b) {
        return *this = *this * b;
    }
};
#endif


template <typename T=int>
BigInt<T> PowInt(BigInt<T> x, uint32_t n) {
    BigInt<T> res;
    uint32_t i = 1;
    for (auto t = n; t; t /= 2, i *= 2);
    for (res = 1; i; i /= 2) {
        res *= res;
        if (n & i) res *= x;
    }
    return res;
}
