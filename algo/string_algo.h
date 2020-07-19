/**
 * @file string_algo.h
 * @brief String specific algorithms.
 */
#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <numeric>


///////////////////////////////////////////////////////////////////////////////////////////////////
//                                       Suffix Array
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Calculate Longest Common Prefix array given original text and Suffix Array.
 */
template<typename T>
std::vector<size_t> LcpSakai(const T* text, size_t n, const std::vector<size_t>& sa) {
   std::vector<size_t> lcp(n, 0);
   std::vector<size_t> rank(n, 0);
   for (size_t i = 0; i < n; i++) rank[sa[i]] = i;

   size_t k = 0;
   for (size_t i = 0; i < n; i++) {
      if (rank[i] == n - 1) {
         k = 0;
         continue;
      }

      auto j = sa[rank[i] + 1];
      while (i + k < n && j + k < n && text[i + k] == text[j + k]) k++;
      lcp[rank[i] + 1] = k;
      if (k > 0) k--;
   }

   return lcp;
}

/**
 * Radix sort with special modification for SA sorting.
 */
template <size_t radix_bits = 8, class T = int>
void SaRadixSort(T* nums, const size_t n, const size_t max_shift) {
   constexpr size_t mask = (1U << radix_bits) - 1U;
   size_t count[1 << radix_bits];
   auto aux_unique = std::make_unique<T[]>(n);
   auto aux = aux_unique.get();

   /* LSD Radix Sort */
   for (size_t shift = 0; shift < max_shift; shift += radix_bits) {
      memset(count, 0, sizeof(count));

      for (size_t i = 0; i < n; i++)    // Counting sort
         count[(nums[i].second >> shift) & mask]++;

      // Calculate buckets positions (cumulative sum)
      for (size_t i = 0; i < mask;) {
         auto temp = count[i++];
         count[i] += temp;
      }

      // Put buckets content into temporary array
      for (size_t i = n; i-- != 0; )
         aux[--count[(nums[i].second >> shift) & mask]] = nums[i];

      std::swap(aux, nums);
   }
}

/**
 * Compute Suffix Array sorted in ascending order.
 * This simple algorithm has time complexity O(n * log(n)) with Radix Sort subroutine.
 * If instead something like std::sort is used, the complexity will be O(n * log(n)^2).
 */
template<typename T>
std::vector<size_t> ComputeSa(const T* s, size_t n) {
   std::vector<size_t> suffix_rank(n);
   size_t shift = 0;
   for (auto t = n; t; t >>= 8, shift += 8);

   for (size_t i = 0; i < n; ++i)
      suffix_rank[i] = s[i];

   std::vector<std::pair<size_t, uint64_t>> fusion(n);

   for (size_t l = 1;;) {
      for (size_t i = 0; i < n; ++i) {
         fusion[i].first = i;
         fusion[i].second = (suffix_rank[i] << shift) | (i + l < n ? suffix_rank[i + l] : 0);
      }

      SaRadixSort(fusion.data(), n, shift * 2);
      l <<= 1;
      if (l >= n) break;

      suffix_rank[fusion[0].first] = 1;
      for (size_t i = 1, curr_rank = 1; i < n; ++i) {
         if (fusion[i - 1].second != fusion[i].second) curr_rank++;
         suffix_rank[fusion[i].first] = curr_rank;
      }
   }

   for (size_t i = 0; i < n; ++i)
      suffix_rank[i] = fusion[i].first;

   return suffix_rank;
}

/**
 * Induced sorting. Key subroutine of SA-IS algorithm.
 */
template<typename T>
void InducedSorting(
   const T* text,
   size_t n,
   T min_item,
   size_t range,
   const std::vector<size_t>& starts,
   const std::vector<bool>& stype,
   const std::vector<size_t>& lms,
   std::vector<size_t>& sa
) {

   // Fill in LMS terms
   {
      std::vector<size_t> starts_copy = starts;
      for (auto i = lms.rbegin(); i != lms.rend(); i++) {
         auto& end = starts_copy[text[*i] - min_item + 1];
         end -= 1;
         sa[end] = *i;
      }
   }

   // Fill in L
   {
      std::vector<size_t> starts_copy = starts;
      // Special case: n-1. We don't use a sentinel.
      {
         auto& start = starts_copy[text[n - 1] - min_item];
         sa[start] = n - 1;
         start += 1;
      }

      for (auto i = 0; i < n; i++) {
         if (sa[i] != n && sa[i] > 0 && !stype[sa[i] - 1]) {
            auto& start = starts_copy[text[sa[i] - 1] - min_item];
            sa[start] = sa[i] - 1;
            start += 1;
         }
      }
   }

   // Fill in S
   {
      std::vector<size_t> starts_copy = starts;
      for (size_t i = n - 1; i < n; i--) {
         if (sa[i] != n && sa[i] > 0 && stype[sa[i] - 1]) {
            auto& end = starts_copy[text[sa[i] - 1] - min_item + 1];
            end -= 1;
            sa[end] = sa[i] - 1;
         }
      }
   }
}

template<typename T>
bool IsEqualLms(const T* text, size_t n, size_t a, size_t b, const std::vector<bool>& stype) {
   if (text[a] != text[b]) {
      return false;
   }
   auto hit_l = false;
   while (true) {
      a++;
      b++;
      if (b < n && stype[b] && a == n) {
         return true;
      }
      if (a < n && stype[a] && b == n) {
         return true;
      }
      if (a == n || b == n || text[a] != text[b] || stype[a] != stype[b]) {
         return false;
      }
      if (stype[a]) {
         if (hit_l) {
            return true;
         }
      }
      else {
         hit_l = true;
      }
   }
}

template<typename T>
static std::vector<bool> ComputeStype(const T* text, size_t n) {
   std::vector<bool> stype(n, false);
   for (size_t i = n - 2; i < n; i--) {
      T a = text[i];
      T b = text[i + 1];
      if (a < b) {
         stype[i] = true;
      }
      else if (a == b) {
         stype[i] = stype[i + 1];
      }
   }

   return stype;
}

/**
 * Compute Suffix Array sorted in ascending order.
 * This is an advanced SA-IS algorithm with time complexity O(n).
 * @see https://ugene.dev/tracker/secure/attachment/12144/Linear+Suffix+Array+Construction+by+Almost+Pure+Induced-Sorting.pdf
 */
template<typename T>
std::vector<size_t> ComputeSaIS(const T* text, size_t n) {
   std::vector<size_t> sa(n, n);
   if (n == 0) return sa;

   // Compute alphabet range
   T min_item = text[0];
   T max_item = text[0];
   for (size_t i = 1; i < n; i++) {
      min_item = std::min(min_item, text[i]);
      max_item = std::max(max_item, text[i]);
   }

   const size_t range = max_item - min_item + 1;

   // Bucketize result
   std::vector<size_t> counts(range, 0);
   size_t needs_sorting = 0;

   for (size_t i = 0; i < n; i++) {
      needs_sorting |= counts[text[i] - min_item]++; // branchless
   }

   if (!needs_sorting) {
      // If all characters are different we can exit
      std::vector<size_t> rev(range, n);
      for (size_t i = 0; i < n; i++) {
         rev[text[i] - min_item] = i;
      }
      for (size_t i = 0, j = 0; i < range; i++) {
         sa[j] = rev[i];
         j += rev[i] < n; // branchless
      }
      return sa;
   }

   std::vector<size_t> starts(range + 1, 0);
   std::partial_sum(counts.begin(), counts.end(), starts.begin() + 1);

   // Separate the list into lms-blocks
   std::vector<bool> stype = ComputeStype(text, n);

   std::vector<size_t> lms;
   for (size_t i = 1; i < n; i++) {
      if (stype[i] && !stype[i - 1]) {
         lms.push_back(i);
      }
   }

   InducedSorting(text, n, min_item, range, starts, stype, lms, sa);

   // Sort lms-suffixes
   if (lms.size() > 1) {
      std::vector<size_t> lms_blocks;
      for (size_t i = 0; i < n; i++) {
         if (sa[i] > 0 && sa[i] != n && stype[sa[i]] && !stype[sa[i] - 1]) {
            // lms
            lms_blocks.push_back(sa[i]);
         }
      }

      // Merge identical LMS blocks
      size_t lms_numbering = 0;
      sa[lms_blocks[0]] = lms_numbering;
      for (size_t i = 1, m = lms_blocks.size(); i < m; i++) {
         auto last = lms_blocks[i - 1];
         auto curr = lms_blocks[i];
         if (!IsEqualLms(text, n, last, curr, stype)) {
            lms_numbering += 1;
         }
         sa[lms_blocks[i]] = lms_numbering;
      }

      std::vector<size_t> sub_problem;
      sub_problem.reserve(lms.size());
      for (auto lm : lms) {
         sub_problem.push_back(sa[lm]);
      }

      auto sub_sa = ComputeSaIS(&*sub_problem.begin(), sub_problem.size());
      std::vector<size_t> lms2;
      lms2.reserve(lms.size());
      for (auto i : sub_sa) {
         lms2.push_back(lms[i]);
      }

      // Reset SA
      std::fill(sa.begin(), sa.end(), n);
      InducedSorting(text, n, min_item, range, starts, stype, lms2, sa);
   }

   return sa;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//                                       String search
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Knuth–Morris–Pratt string search algorithm.
 * @see https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm
 */
template<typename T>
int strStrKMP(const T *text, size_t n, const T *s, size_t m) {
   if (m == 0) return 0;
   if (m >= n) return -1;
   vector<int> pi(--m);

   int k = 0, i = 1;
   while (i < m) {
      if (s[k] == s[i]) {
         pi[i++] = ++k;
      }
      else if (k > 0) {
         k = pi[k - 1];
      }
      else ++i;
   }

   for (k = 0, i = 0; i < n; ) {
      if (s[k] == text[i]) {
         if (k++ == m) return i - m;
         i++;
      }
      else if (k > 0) {
         k = pi[k - 1];
      }
      else ++i;
   }

   return -1;
}

/**
 * Boyer-Moore string search algorithm.
 * @see https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string-search_algorithm
 */
template<typename T>
int strStrBM(const T *text, size_t n, const T *s, size_t m) {
   if (m == 0) return 0;
   if (m > n) return -1;
   vector<size_t> z(m);

   for (size_t i = 1, l = 0, r = 0; i < m; ++i) {
      if (i < r) {
         z[i] = min(z[i - l], r - i);
      }
      while (z[i] + i < m && s[m - 1 - z[i]] == s[m - 1 - (i + z[i])]) {
         z[i]++;
      }
      if (i + z[i] > r) {
         l = i;
         r = i + z[i];
      }
   }

   vector<size_t> suffshift(m + 1);
   for (size_t i = 0; i <= m; ++i) suffshift[i] = m;
   for (size_t i = m; --i;) suffshift[m - z[i]] = i;
   for (size_t i = 1, r = 0; i < m; ++i) {
      if (i + z[i] == m) {
         for (; r <= i; r++) {
            if (suffshift[r] == m) {
               suffshift[r] = i;
            }
         }
      }
   }

   int j, bound = 0; // always bound = 0 or bound = m - suffshift[0]
   for (int i = 0; i <= n - m; i += suffshift[j + 1]) {
      for (j = m - 1; j >= bound && s[j] == text[i + j]; --j);
      if (j < bound) {
#if 0
         report_occurrence(i);
         bound = m - suffshift[0];
         j = -1;
#else
         return i;
#endif
      }
      else {
         bound = 0;
      }
   }

   return -1;
}
