#include <bits/stdc++.h>
#include <gtest/gtest.h>

#include "sorting.h"

using namespace std;


/* Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.  */

double timespec_subtract(struct timespec &x, struct timespec &y, struct timespec &diff) {
   constexpr long ns_in_s = 1'000'000'000L;
   diff.tv_sec = x.tv_sec - y.tv_sec;
   diff.tv_nsec = x.tv_nsec - y.tv_nsec;
   if (diff.tv_nsec < 0) {
      diff.tv_sec--;
      diff.tv_nsec += ns_in_s;
   }
   auto time_diff = static_cast<double>(x.tv_sec - y.tv_sec) * 1e9;
   return (time_diff + static_cast<double>(x.tv_nsec - y.tv_nsec)) * 1e-9;
}


TEST(Sorting, Radix) {
   for (size_t sort_size = 100; sort_size <= 100000000; sort_size *= 10) {
      cout << "Comparing speed of RadixSort to std::sort\n";
      cout << "Array size: " << sort_size << endl;
      vector<int> nums(100000);
      random_device rd;  //Will be used to obtain a seed for the random number engine
      mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
      uniform_int_distribution<> dis(INT_MIN, INT_MAX);

      for (auto & i : nums) i = dis(gen);
      auto nums_copy = nums;

      struct timespec start = {0, 0}, end = {0, 0};
      auto diff = end;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
      RadixSort(nums.data(), nums.size());
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
      auto radix_time = timespec_subtract(end, start, diff);
      cout << "Radix sort time elapsed: " << diff.tv_sec << "s " << diff.tv_nsec << "ns\n";

      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
      std::sort(nums_copy.begin(), nums_copy.end());
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
      auto std_time = timespec_subtract(end, start, diff);
      cout << "Standard sort time elapsed: " << diff.tv_sec << "s " << diff.tv_nsec << "ns\n";

      // speed is reverse proportional to time taken
      cout << "radix_speed / std_speed = " << std_time / radix_time << endl;

      EXPECT_EQ(0, memcmp(nums.data(), nums_copy.data(), sizeof(nums[0]) * nums.size()));

      cout << endl;
   }
}

int main(int argc, char **argv) {
   testing::InitGoogleTest(&argc, argv);
   ios_base::sync_with_stdio(false);
   cin.tie(nullptr);
   cout.tie(nullptr);

   return RUN_ALL_TESTS();
}