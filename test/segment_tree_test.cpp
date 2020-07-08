/**
 * @file segment_tree_test.cpp
 * @brief Tests for segment and fenwick trees.
 */
#include <algorithm>
#include <gtest/gtest.h>

#include "segment_tree.h"

using namespace std;


/**
 * Modified RadixSort, which is able to sort an array of complex objects.
 * Namely pair<int64_t, size_t>.
 */
template <size_t radix_bits = 8, class T = int>
void RadixSort(T* nums, const size_t n) {
   constexpr size_t mask = (1U << radix_bits) - 1U;
   size_t count[1U << radix_bits];
   auto aux_unique = std::make_unique<T[]>(n);
   auto aux = aux_unique.get();
   constexpr size_t num_size = sizeof(int64_t);

   /* LSD Radix Sort */
   for (size_t shift = 0; shift < num_size * 8; shift += radix_bits) {
      memset(count, 0, sizeof(count));

      for (size_t i = 0; i < n; i++)    // Counting sort
         count[static_cast<size_t>(nums[i].first >> shift) & mask]++;

      // Calculate buckets positions (cumulative sum)
      if (shift + radix_bits < num_size * 8) {
         for (size_t i = 0; i < mask;) {
            auto temp = count[i++];
            count[i] += temp;
         }
      }
      else { // sort signed integers
         for (size_t i = (mask >> 1U) + 1U; i != (mask >> 1U);) {
            auto temp = count[i++];
            i &= mask;
            count[i] += temp;
         }
      }

      // Put buckets content into temporary array
      for (size_t i = n; i-- != 0; ) {
         aux[--count[static_cast<size_t>(nums[i].first >> shift) & mask]] = nums[i];
      }

      std::swap(aux, nums);
   }
}


int count_ranges(vector<pair<int64_t, size_t>>& arr, int limit){
   FenwickTree<int> ft(arr.size());
   int result = 0;
   for (auto i = arr.size(), j = i; i--; ) {
      for (; j != 0 && arr[i].first - arr[j - 1].first <= limit; --j) {
         ft.Update(arr[j - 1].second, 1);
      }
      result += ft.Query(0, arr[i].second - 1UL);
   }
   return result;
}

/**
 * @brief Count of Range Sum
 * Given an integer array nums, return the number of range sums that lie in
 * [lower, upper] inclusive. Range sum S(i, j) is defined as the sum of the
 * elements in nums between indices i and j (i ≤ j), inclusive.
 *
 * @note This problem is from leetcode.com
 */
int countRangeSum(vector<int>& nums, int lower, int upper) {
   int64_t sum = 0;
   auto n = nums.size();
   if (n <= 0 || lower > upper) {
      return 0;
   }
   vector<pair<int64_t, size_t>> prefix_sums;
   prefix_sums.reserve(n + 1);
   prefix_sums.emplace_back(0, 0);
   for (size_t i = 0; i < n;){
      sum += nums[i];
      prefix_sums.emplace_back(sum, ++i);
   }
   RadixSort(prefix_sums.data(), prefix_sums.size());

   int upper_result = count_ranges(prefix_sums, upper);
   int lower_result = count_ranges(prefix_sums, lower - 1);
   return max(upper_result - lower_result, 0);
}


TEST(SegmentTree, FenwickTree) {
   vector<int> nums = {
   -68, 144, 113, -29, 135, -69, 121, -64, 106, 116, 120, 91, -44, 26, 66, -90,
    147, 71, 86, 163, -76, 50, 179, 165, 182, 62, 163, 172, 113, 109, 162, -52,
    -83, -52, 31, -64, 13, -83, -99, 59, -94, 55, 158, 146, 1, 76, 57, -92, -21,
    89, 98, 162, 15, 70, 181, 28, 149, 143, 55, 88, 49, 145, 159, -9, 1, -26, -18,
    131, 172, 140, -62, -14, 89, -92, 93, 135, 130, 116, 8, 173, -78, 80, 15, -35,
    186, 16, 92, -60, -29, -52, -96, -28, 155, 35, -1, 89, 74, 59, 49, -57, -95,
    0, 60, 38, 122, 89, -68, 181, 176, 2, 36, 17, 192, 18, 35, 23, 168, 178, -48,
    164, 186, 200, -33, -14, 100, -54, 60, -70, 23, 128, -62, 157, 120, -2, -18,
    -80, 27, 13, 107, -92, 112, 94, 187, 133, 14, -78, -90, 48, 96, 131, -23, -47,
    -88, -13, -30, 112, -77, 34, -33, -38, -8, 153, 183, -21, 24, -52, 124, 161,
    33, 145, 35, -70, 86, -59, -21, 145, -90, 190, -33, 138, -87, 158, -20, 153,
    1, 79, 105, 164, 113, -79, -77, 90, 125, -73, 77, 41, -51, 14, 110, 90 };
   int res = countRangeSum(nums, -10, 10);
   EXPECT_EQ(res, 48);
}
