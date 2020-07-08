/**
 * @file sorting_test.cpp
 * @brief Tests for radix sort.
 */
#include <random>
#include <fstream>
#include <chrono>
#include <gtest/gtest.h>

#include "sorting.h"

using namespace std;


template <class T, class Distribution, class Generator>
void RadixBenchmark(Distribution dis, Generator &gen, const char* type_name, ofstream &plot_data, bool reverse = false) {
   cout << "\nComparing speed of RadixSort with std::sort\n";
   cout << "Array element type: " << type_name << endl;
   plot_data << "RadixSort vs std::sort (" << type_name << ")" << endl;
   vector<int64_t> radix_plot, std_plot;

   constexpr size_t sort_start = 100, sort_end = 10'000'000;
   for (size_t sort_size = sort_start; sort_size <= sort_end; sort_size *= 10) {
      cout << "Array size: " << sort_size << endl;
      vector<T> nums(sort_size);

      for (auto & i : nums) i = dis(gen);
      auto nums_copy = nums;

      auto start = chrono::high_resolution_clock::now();
      RadixSort(nums.data(), nums.size(), reverse);
      auto end = chrono::high_resolution_clock::now();
      auto radix_time = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
      cout << "Radix sort time elapsed: " << radix_time << "ns\n";

      start = chrono::high_resolution_clock::now();
      if (reverse) {
         std::sort(nums_copy.begin(), nums_copy.end(), std::greater<T>());
      }
      else {
         std::sort(nums_copy.begin(), nums_copy.end());
      }
      end = chrono::high_resolution_clock::now();
      auto std_time = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
      cout << "Standard sort time elapsed: " << std_time << "ns\n";

      // speed is reverse proportional to time taken
      cout << "radix_speed / std_speed = " << (double)std_time / radix_time << endl;
      radix_plot.push_back(radix_time);
      std_plot.push_back(std_time);

      EXPECT_EQ(0, memcmp(nums.data(), nums_copy.data(), sizeof(nums[0]) * nums.size()));

      cout << endl;
   }

   plot_data << "RadixSort" << endl;
   for (size_t sort_size = sort_start; sort_size <= sort_end; sort_size *= 10)
      plot_data << sort_size << " ";
   plot_data << endl;
   for (auto n : radix_plot) plot_data << n << " ";
   plot_data << endl;
   plot_data << "std::sort" << endl;
   for (size_t sort_size = sort_start; sort_size <= sort_end; sort_size *= 10)
      plot_data << sort_size << " ";
   plot_data << endl;
   for (auto n : std_plot) plot_data << n << " ";
   plot_data << endl << endl;

   cout << endl << endl;
}


TEST(Sorting, Radix) {
   random_device rd;  //Will be used to obtain a seed for the random number engine
   mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
   ofstream plot_data("plot_data.txt");

   RadixBenchmark<int>(uniform_int_distribution<int>(INT_MIN, INT_MAX), gen, "int", plot_data);
   RadixBenchmark<uint32_t>(uniform_int_distribution<uint32_t>(0U, UINT_MAX), gen, "uint32_t", plot_data);
   RadixBenchmark<int64_t>(uniform_int_distribution<int64_t>(INT64_MIN, INT64_MAX), gen, "int64_t", plot_data, true);
   RadixBenchmark<uint64_t>(uniform_int_distribution<uint64_t>(0UL, UINT64_MAX), gen, "uint64_t", plot_data);
   RadixBenchmark<float>(uniform_real_distribution<float>(-1e30f, 1e30f), gen, "float", plot_data);
   RadixBenchmark<float>(uniform_real_distribution<float>(-1.f, 1.f), gen, "float", plot_data, true);
   RadixBenchmark<double>(uniform_real_distribution<double>(-1e100, 1e100), gen, "double", plot_data, true);
   RadixBenchmark<double>(uniform_real_distribution<double>(-1., 1.), gen, "double", plot_data);
}
