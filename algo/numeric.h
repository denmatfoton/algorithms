/**
 * @file numeric.h
 * @brief Numbers and math specific algorithms.
 */
#pragma once
#include <vector>
#include <random>
#include <sstream>
#include <algorithm>
#include <unordered_map>
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

/**
 * Ceil number to the nearest power of 2.
 */
template<typename T, bool use_intrinsics = true>
T CeilToPow2(T x) {
    // using intrinsic instructions should be faster
    if constexpr (use_intrinsics) {
        if (!x) return x;
        int s = 0;
        if constexpr (sizeof(T) == sizeof(unsigned long long)) {
            if (__builtin_popcountll(x) == 1) return x;
            s = sizeof(T) * 8 - __builtin_clzll(x);
        }
        else if constexpr (sizeof(T) == sizeof(unsigned long)) {
            if (__builtin_popcountl(x) == 1) return x;
            s = sizeof(T) * 8 - __builtin_clzl(x);
        }
        else {
            if (__builtin_popcount(x) == 1) return x;
            s = sizeof(T) * 8 - __builtin_clz(x);
        }
        return T(1) << s;
    }
    else {
        --x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        for (T i = 1; i < sizeof(T); i <<= 1) {
            x |= x >> (i << 3);
        }
        ++x;
    }
    return x;
}

template <typename T=uint32_t>
T MySqrt(T x) {
    T a = x;
    T temp = x;
    while (x) {
        a >>= 1;
        x >>= 2;
    }
    a |= 1;
    while ((a + 1) * (a + 1) <= temp || a * a > temp) {
        a = (temp / 2 + a * a / 2) / a;
    }
    return a;
}

template<typename T>
T Gcd(T a, T b) {
    while (b != 0) {
        T r = a % b;
        a = b;
        b = r;
    }
    return a;
}

template<typename T>
T ExtendedGcd(T a, T b, T* x, T* y) {
   T s = 0, old_s = 1;
   T r = b, old_r = a;

   while (r != 0) {
      T q = old_r / r;
      T temp = r;
      r = old_r - q * r;
      old_r = temp;
      temp = s;
      s = old_s - q * s;
      old_s = temp;
   }

   *x = old_s;
   *y = b ? (old_r - old_s * a) / b : 0;
   return old_r;
}

/**
 * Calculate modular inverse of a number a given module m.
 */
template<typename T>
T ModInverse(T a, T m) {
    T x = 0, y = 0;
    ExtendedGcd(a, m, &x, &y);
    if (x < 0) x += m;
    return x;
}

/**
 * Factorize a number into its prime factors.
 */
template <typename T=uint64_t>
std::vector<std::pair<T, size_t>> Factorize(T n) {
    std::vector<std::pair<T, size_t>> out;
    for (T i = 2; i * i <= n; ++i) {
        for (bool add = true; n % i == 0; n /= i) {
            if (add) {
                add = false;
                out.emplace_back(i, 0);
            }
            out.back().second++;
        }
    }
    if (n > 1) out.emplace_back(n, 1);
    return out;
}


/////////////////////////////////////////////////////////////
// Prime numbers
/////////////////////////////////////////////////////////////

/**
 * @note Don't use template type other than default.
 */
template <typename T=uint32_t>
bool is_SPRP(T n, T a) {
    T d = n - 1, s = 0;
    while ((d & 1) == 0) ++s, d >>= 1;
    uint64_t cur = 1;
    while (d) {
        if (d & 1) cur = (cur * a) % n;
        a = ((uint64_t)a * a) % n;
        d >>= 1;
    }
    if (cur == 1) return true;
    for (T r = 0; r < s; r++) {
        if (cur == n - 1) return true;
        cur = (cur * cur) % n;
    }
    return false;
}

/**
 * Optimized primality test for 32-bit numbers
 * @see http://ceur-ws.org/Vol-1326/020-Forisek.pdf
 * @note Don't use template type other than default.
 */
template <typename T=uint32_t>
bool IsPrimeSPRP32(T x) {
    static const uint16_t bases[] = { 15591,2018,166,7429,8064,16045,10503,4399,1949,1295,2776,3620,560,3128,5212,
                     2657,2300,2021,4652,1471,9336,4018,2398,20462,10277,8028,2213,6219,620,3763,4852,5012,3185,
                     1333,6227,5298,1074,2391,5113,7061,803,1269,3875,422,751,580,4729,10239,746,2951,556,2206,
                     3778,481,1522,3476,481,2487,3266,5633,488,3373,6441,3344,17,15105,1490,4154,2036,1882,1813,
                     467,3307,14042,6371,658,1005,903,737,1887,7447,1888,2848,1784,7559,3400,951,13969,4304,177,41,
                     19875,3110,13221,8726,571,7043,6943,1199,352,6435,165,1169,3315,978,233,3003,2562,2994,10587,
                     10030,2377,1902,5354,4447,1555,263,27027,2283,305,669,1912,601,6186,429,1930,14873,1784,1661,
                     524,3577,236,2360,6146,2850,55637,1753,4178,8466,222,2579,2743,2031,2226,2276,374,2132,813,
                     23788,1610,4422,5159,1725,3597,3366,14336,579,165,1375,10018,12616,9816,1371,536,1867,10864,
                     857,2206,5788,434,8085,17618,727,3639,1595,4944,2129,2029,8195,8344,6232,9183,8126,1870,3296,
                     7455,8947,25017,541,19115,368,566,5674,411,522,1027,8215,2050,6544,10049,614,774,2333,3007,
                     35201,4706,1152,1785,1028,1540,3743,493,4474,2521,26845,8354,864,18915,5465,2447,42,4511,
                     1660,166,1249,6259,2553,304,272,7286,73,6554,899,2816,5197,13330,7054,2818,3199,811,922,350,
                     7514,4452,3449,2663,4708,418,1621,1171,3471,88,11345,412,1559,194 };
    if (x == 2 || x == 3 || x == 5 || x == 7) return true;
    if (x % 2 == 0 || x % 3 == 0 || x % 5 == 0 || x % 7 == 0) return false;
    if (x < 121) return (x > 1);
    uint64_t h = x;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h) & 255;
    return is_SPRP(x, bases[h]);
}

/**
 * Calculate modular power of a number.
 */
template <typename T=uint64_t>
T PowMod(T x, T p, T m) {
    T i = 1;
    for (auto t = p; t; t /= 2, i *= 2);
    uint64_t res = 1;
    if constexpr (sizeof(T) <= 4) {
        for (; i; i /= 2) {
            res = (res * res) % m;
            if (p & i) res = (res * x) % m;
        }
    }
    else {
        for (; i; i /= 2) {
#if defined(_MSC_VER)
            uint64_t h = 0;
            auto l = _umul128(res, res, &h);
            _udiv128(h, l, m, &res);
            if (p & i) {
                auto l = _umul128(res, x, &h);
                _udiv128(h, l, m, &res);
            }
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
            res = ((unsigned __int128)(res) * res) % m;
            if (p & i) res = ((unsigned __int128)(res) * x) % m;
#else
#error Unsupported compiller, 128 bit arithmetic required
#endif
        }
    }
    return res;
}

/**
 * Miller–Rabin primality test.
 */
template <typename T=uint32_t, size_t lim=1'000'000>
bool IsPrimeMR(T n, uint32_t k = 7) {
    static bool not_prime[lim] = { true, true, false };
    // fill the array once
    if (!not_prime[4]) {
        for (size_t i = 2; i <= sqrt(lim); ++i) {
            if (not_prime[i]) continue;
            for (size_t j = i * 2; j < lim; j += i) {
                not_prime[j] = true;
            }
        }
    }
    if (n < lim) return !not_prime[n];
    if (n < UINT32_MAX) return IsPrimeSPRP32(uint32_t(n));
    // deterministic 
    static const uint64_t *arr[3] = {
        { 2, 7, 61 }, // < 4'759'123'141
        { 2, 13, 23, 1662803 }, // < 1'122'004'669'633
        { 2, 3, 5, 7, 11, 13, 17 }, // < 341'550'071'728'321
        { 2, 325, 9375, 28178, 450775, 9780504, 1795265022 } // all 64-bit
    };
    int idx = 3, l = 7;
    if (n < 4'759'123'141) {
        idx = 0;
        l = 3;
    }
    else if (n < 1'122'004'669'633) {
        idx = 1;
        l = 4;
    }
    else if (n < 341'550'071'728'321) {
        idx = 2;
    }
    T d = n - 1, s = 0;
    for (; !(d & 1); d >>= 1, ++s);
    for (auto a : arr) {
        auto x = PowMod(a, d, n);
        if (x != 1 && x != n - 1) {
            T r = 0;
            for (; ++r < s;) {
#if defined(_MSC_VER)
                uint64_t h = 0;
                auto l = _umul128(x, x, &h);
                _udiv128(h, l, n, &x);
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
                x = ((unsigned __int128)(x) * x) % n;
#else
#error Unsupported compiller, 128 bit arithmetic required.
#endif
                if (x == 1) return false;
                if (x == n - 1) break;
            }
            if (r == s) return false;
        }
    }
    return true;
}


template <typename T=uint32_t>
std::vector<T> CalcSqrtFraction(T x) {
    std::vector<T> out;
    auto q = MySqrt(x);
    out.push_back(q);
    if (q * q == x) return out;
    T n = 1, r = q;
    while (true) {
        auto d = x - r * r;
        d /= n;
        auto a = (q + r) / d;
        out.push_back(a);
        r = a * d - r;
        n = d;
        if (n == 1 && r == q) break;
    }
    return out;
}
