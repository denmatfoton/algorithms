/**
 * @file test.cpp
 * @brief Testing start point.
 */
#include <iostream>
#include <gtest/gtest.h>


int main(int argc, char **argv) {
   testing::InitGoogleTest(&argc, argv);
   // for speed purposes
   std::ios_base::sync_with_stdio(false);
   std::cin.tie(nullptr);
   std::cout.tie(nullptr);

   return RUN_ALL_TESTS();
}
