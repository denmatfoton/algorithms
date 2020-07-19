/**
 * @file sorting.h
 * @brief Template sorting algorithms.
 */
#pragma once

#include <memory>
#include <vector>
#include <queue>


/**
 * Radix sort works for different types of integers.
 * Linear time complexity: O[ (n + (1 << radix_bits)) * (sizeof(T) << 3) / radix_bits ].
 * Uses O[ n + (1 << radix_bits) ] extra space.
 * @tparam radix_bits Number of bits of element to be processed in outer loop iteration.
 * @tparam T Array element type.
 * @param nums Pointer to array to be sorted.
 * @param n Size of the array.
 * @param reverse If true sorts in descending order.
 * @note It is efficient on big arrays. On small arrays (n < 100) std::sort might be faster.
 * @note It is a stable sorting algorithm.
 */
template <size_t radix_bits = 8, class T = int>
void RadixSort(T* nums, const size_t n, bool reverse = false) {
   static_assert((sizeof(T) * 8) % (radix_bits * 2) == 0); // size should be divisible by radix * 2
   constexpr size_t mask = (1U << radix_bits) - 1U;
   size_t count[1U << radix_bits];
   auto aux_unique = std::make_unique<T[]>(n);
   auto aux = aux_unique.get();

   /* LSD Radix Sort */
   for (size_t shift = 0; shift < sizeof(T) * 8; shift += radix_bits) {
      memset(count, 0, sizeof(count));

      for (size_t i = 0; i < n; i++)    // Counting sort
         count[static_cast<size_t>(nums[i] >> shift) & mask]++;

      // Calculate buckets positions (cumulative sum)
      if constexpr (std::is_signed<T>::value) {
         if (shift + radix_bits < sizeof(T) * 8) {
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
      if (reverse && shift + radix_bits == sizeof(T) * 8) {
         for (size_t i = n; i-- != 0;) {
            auto idx = static_cast<size_t>(nums[i] >> shift) & mask;
            aux[n - count[idx]--] = nums[i];
         }
      }
      else {
         for (size_t i = n; i-- != 0;) {
            auto idx = static_cast<size_t>(nums[i] >> shift) & mask;
            aux[--count[idx]] = nums[i];
         }
      }

      std::swap(aux, nums);
   }
}

/**
 * Radix sort works with float and double.
 * Linear time complexity: O[ (n + (1 << radix_bits)) * (sizeof(T) << 3) / radix_bits ].
 * Uses O[ n + (1 << radix_bits) ] extra space.
 * @tparam radix_bits Number of bits of element to be processed in outer loop iteration.
 * @tparam T Array element type.
 * @param nums Pointer to array to be sorted.
 * @param n Size of the array.
 * @param reverse If true sorts in descending order.
 * @note It is efficient on big arrays. On small arrays (n < 100) std::sort might be faster.
 * @note It is a stable sorting algorithm.
 */
template <size_t radix_bits = 8, class T>
void FloatRadixSort(T* int_cast_nums, const size_t n, bool reverse = false) {
   constexpr size_t mask = (1U << radix_bits) - 1U;
   size_t count[1U << radix_bits];
   auto aux_unique = std::make_unique<T[]>(n);
   auto aux = aux_unique.get();

   /* LSD Radix Sort */
   for (size_t shift = 0; shift < sizeof(T) * 8; shift += radix_bits) {
      memset(count, 0, sizeof(count));

      for (size_t i = 0; i < n; i++)    // Counting sort
         count[static_cast<size_t>(int_cast_nums[i] >> shift) & mask]++;

      // Calculate buckets positions (cumulative sum)
      for (size_t i = 0; i < mask;) {
         auto temp = count[i++];
         count[i] += temp;
      }

      if (shift + radix_bits < sizeof(T) * 8) {
         // Put buckets content into temporary array
         for (size_t i = n; i-- != 0;) {
            auto idx = static_cast<size_t>(int_cast_nums[i] >> shift) & mask;
            aux[--count[idx]] = int_cast_nums[i];
         }
      }
      // sort exponent
      else if (reverse) {
         auto pos_num = count[mask >> 1U];
         for (size_t i = n; i--;) {
            auto idx = static_cast<size_t>(int_cast_nums[i] >> shift) & mask;
            if (idx > mask >> 1U) {
               aux[--count[idx]] = int_cast_nums[i];
            }
            else {
               aux[pos_num - count[idx]--] = int_cast_nums[i];
            }
         }
      }
      else {
         auto neg_num = n - count[mask >> 1U];
         for (size_t i = n; i--;) {
            auto idx = static_cast<size_t>(int_cast_nums[i] >> shift) & mask;
            if (idx > mask >> 1U) {
               // negative numbers go in buckets in reverse order
               aux[n - count[idx]--] = int_cast_nums[i];
            }
            else {
               aux[neg_num + --count[idx]] = int_cast_nums[i];
            }
         }
      }

      std::swap(aux, int_cast_nums);
   }
}

template <>
void RadixSort<8, float>(float* nums, const size_t n, bool reverse) {
   FloatRadixSort(reinterpret_cast<uint32_t*>(nums), n, reverse);
}

template <>
void RadixSort<8, double>(double* nums, const size_t n, bool reverse) {
   FloatRadixSort(reinterpret_cast<uint64_t*>(nums), n, reverse);
}


/**
 * Sorting of any type of comparable date using Cartesian Tree.
 *
 * Unlike Heap, Cartesian sort makes use of the fact that the data is partially sorted.
 * This gives a linear complexity when the data is partially sorted.
 * O(n * log(n)) in worst case.
 * Uses linear amount of additional memory.
 *
 * @tparam T Array element type.
 * @tparam _Cmp type of compatison operator.
 * @param arr Pointer to the array to be sorted.
 * @param n Size of the array.
 * @param cmp compatison operator for type T.
 * If cmp compares less <, array will be sorted in ascending order.
 * @note It is NOT a stable sorting algorithm.
 */
template<typename T, typename _Cmp = std::less<T>>
std::vector<T> CartesianSort(const T* arr, size_t n, _Cmp cmp = _Cmp()) { 
    // Arrays to hold the index of parent, left-child, 
    // right-child of each number in the input array 
    std::vector<int> parent(n, -1), left_child(n, -1), right_child(n, -1);
  
    // 'root' and 'last' stores the index of the root and the 
    // last processed of the Cartesian Tree. 
    // Initially we take root of the Cartesian Tree as the 
    // first element of the input array. This can change 
    // according to the algorithm 
    int root = 0, prev;
  
    // Starting from the second element of the input array 
    // to the last on scan across the elements, adding them 
    // one at a time. 
    for (int i = 0; ++i < n;) { 
        prev = i - 1;
        // Scan upward from the node's parent up to 
        // the root of the tree until a node is found 
        // whose value is greater than the current one 
        // This is the same as Step 2 mentioned in the 
        // algorithm 
        while (!cmp(arr[prev], arr[i]) && prev != root) 
            prev = parent[prev]; 
  
        // arr[i] is the largest element yet; make it 
        // new root 
        if (!cmp(arr[prev], arr[i])) { 
            left_child[i] = root; 
            root = parent[root] = i; 
        } 
        // Just insert it 
        else if (right_child[prev] == -1) { 
            right_child[prev] = i; 
            parent[i] = prev; 
            left_child[i] = -1; 
        } 
        // Reconfigure links 
        else { 
            parent[right_child[prev]] = i;
            left_child[i] = right_child[prev]; 
            right_child[prev] = i; 
            parent[i] = prev; 
        }
    }

    vector<T> out(n);
    auto cmp_pq = [arr, cmp] (int i, int j) { return !cmp(arr[i], arr[j]); };
    priority_queue<int, vector<int>, decltype(cmp_pq)> pq(cmp_pq); 
    pq.push(root);
  
    size_t i = 0;
    while (!pq.empty()) {
        auto cur = pq.top(); pq.pop();
        out[i++] = arr[cur];
        if (left_child[cur] != -1) pq.push(left_child[cur]); 
        if (right_child[cur] != -1) pq.push(right_child[cur]); 
    }
    return out;
}
