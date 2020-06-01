/**
 * @file sorting.h
 * @brief Template sorting algorithms.
 */
#pragma once

#include <memory>

/**
 * Radix sort works for different types of integers.
 * Linear time complexity: O[ (n + (1 << radix_bits)) * (sizeof(T) << 3) / radix_bits ].
 * Uses O[ n + (1 << radix_bits) ] extra space.
 * @tparam radix_bits Number of bits of element to be processed in outer loop iteration.
 * @tparam T Array element type.
 * @param nums Pointer to array to be sorted.
 * @param n Size of the array.
 * @note It is efficient on big arrays. On small arrays (n < 256) std::sort is faster.
 */
template <size_t radix_bits = 8, class T = int>
void RadixSort(T* nums, const size_t n) {
   static_assert((sizeof(T) * 8) % (radix_bits * 2) == 0); // size should be divisible by radix * 2
   constexpr size_t mask = (1U << radix_bits) - 1U;
   size_t count[1U << radix_bits];
   auto aux_unique = std::make_unique<T[]>(n);
   auto aux = aux_unique.get();

   /* LSD Radix Sort */
   for (size_t shift = 0; shift < sizeof(T) * 8; shift += radix_bits) {
      memset(count, 0, sizeof(count));

      for (size_t i = 0; i < n; i++)    // Counting sort
         count[(nums[i] >> shift) & mask]++;

      // Calculate buckets positions (cumulative sum)
      if constexpr (std::is_signed<T>::value) {
         if (shift + radix_bits < sizeof(T) << 3U) {
            for (size_t i = 0; i < mask;) {
               auto temp = count[i++];
               count[i] += temp;
            }
         }
         else { // sort signed integers
            // start from lowest negative, i.e. 0b1000'0000
            // increment until it overflows, starts from 0 and goes through all positives
            // up to i.e. 0b0111'1111
            for (size_t i = (mask >> 1U) + 1U; i != (mask >> 1U);) {
               auto temp = count[i++];
               i &= mask;
               count[i] += temp;
            }
         }
      }
      else {
         for (size_t i = 0; i < mask;) {
            auto temp = count[i++];
            count[i] += temp;
         }
      }

      // Put buckets content into temporary array
      for (size_t i = n; i-- != 0; ) {
         aux[--count[(nums[i] >> shift) & mask]] = nums[i];
      }

      std::swap(aux, nums);
   }
}
