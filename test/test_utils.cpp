/**
 * @file test_utils.cpp
 * @brief Auxiliary functions for testing.
 */
#include "test_utils.h"


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
