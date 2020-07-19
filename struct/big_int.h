#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>


template <typename T=int>
class BigInt {
   void Trim() {
      while (val.size() && val.back() == 0) val.pop_back();
      if (val.empty()) {
         sig = 0;
         val.push_back(0);
      }
   }

   static std::vector<T> Sub(const std::vector<T> &a, const std::vector<T> &b) {
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

   static std::vector<T> Add(const std::vector<T> &a, const std::vector<T> &b, size_t as = 0) {
       return Add(a.data(), b.data(), a.size(), b.size(), as);
   }

   static std::vector<T> Multiply(const std::vector<T> &a, const std::vector<T> &b) {
      return Multiply(a.data(), b.data(), a.size(), b.size());
   }

   static std::vector<T> Multiply(const T* a, const T* b, size_t al, size_t bl) {
      for (;al; al--) if (a[al - 1]) break;
      for (;bl; bl--) if (b[bl - 1]) break;
      if (al < bl) {
         std::swap(a, b);
         std::swap(al, bl);
      }

      if (bl == 0) return { 0 };
      if (bl <= 2) {
         int64_t carry = 0;
         std::vector<T> res0(al);
         for (size_t i = 0; i < al; ++i, carry /= base) {
            carry += (int64_t)a[i] * b[0];
            res0[i] = carry % base;
         }
         if (carry) res0.push_back(carry);
         if (bl == 1) return res0;
         std::vector<T> res1(al);
         carry = 0;
         for (size_t i = 0; i < al; ++i, carry /= base) {
            carry += (int64_t)a[i] * b[1];
            res1[i] = carry % base;
         }
         if (carry) res1.push_back(carry);
         return Add(res1, res0, 1);
      }

      T l = al / 2;
      T ha = al - l;
      T hb = bl - l;
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
   static constexpr T base = 1e9;
public:
   BigInt() {}

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
      size_t i = n % 9, v = 0;
      for (size_t j = 0; j < i; j++)
         v = v * 10 + (s[j] - '0');
      val.push_back(v);
      for (; i < n; i += 9) {
         v = 0;
         for (auto j = i; j < i + 9; j++)
            v = v * 10 + (s[j] - '0');
         val.push_back(v);
      }
      reverse(val.begin(), val.end());
   }

   BigInt(const std::string &s) : BigInt(s.c_str()) {}

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
       char *s = &out[0] + i;
       for (i = val.size() - 1; i--; s += 9)
           snprintf(s, 10, "%09d", val[i]);
       return out;
   }

   friend std::ostream & operator << (std::ostream &out, const BigInt &a) {
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
      return b > *this;
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


   BigInt operator + (const BigInt &b) const {
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

   BigInt & operator ++ () {
       return *this = *this + 1;
   }

   BigInt operator ++ (int) {
      auto old = *this;
      *this = *this + 1;
      return old;
   }

   BigInt & operator += (const BigInt& b) {
       return *this = *this + b;
   }

   BigInt & operator += (long b) {
       return *this = *this + b;
   }


   BigInt operator - (const BigInt &b) const {
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

   BigInt & operator -- () {
       return *this = *this - 1;
   }

   BigInt operator -- (int) {
      auto old = *this;
      *this = *this - 1;
      return old;
   }

   BigInt & operator -= (const BigInt& b) {
       return *this = *this - b;
   }

   BigInt & operator -= (long b) {
       return *this = *this - b;
   }
   

   size_t DigCount() {
       size_t cnt = (val.size() - 1) * 9;
       for (auto t = val.back(); t; t /= 10, cnt++);
       return cnt;
   }


   BigInt operator * (const BigInt &b) const {
       BigInt ans;
       ans.sig = sig ^ b.sig;
       ans.val = Multiply(val, b.val);
       ans.Trim();
       return ans;
   }

   BigInt operator * (long b) {
       return *this * BigInt(b);
   }

   BigInt & operator *= (const BigInt& b) {
       return *this = *this * b;
   }

   BigInt & operator *= (long b) {
       return *this = *this * b;
   }
};
