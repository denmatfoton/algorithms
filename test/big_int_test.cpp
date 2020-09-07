/**
 * @file sorting_test.cpp
 * @brief Tests for radix sort.
 */
#include <chrono>
#include <gtest/gtest.h>

#include "big_int.h"

using namespace std;


TEST(BigInt, General) {
    BigInt a = "81134792846872691";
    BigInt b = "1454848451848469789";
    EXPECT_TRUE((a + b) == BigInt("1535983244695342480"));
    auto sum = (a + b).ToString();
    EXPECT_EQ(sum, "1535983244695342480");
    EXPECT_TRUE((a - b) == BigInt("-1373713659001597098"));
    EXPECT_TRUE((a * b) == BigInt("118038827764319035262896422442632199"));
    a = "1208925819614629174706174"; // 2 ^ 80 - 2
    b = "1180591620717411303423"; // 2 ^ 70 - 1
    EXPECT_TRUE((a + b) == BigInt("1210106411235346586009597"));
    EXPECT_TRUE((a * b) == BigInt("1427247692705959881057074682446639072385433602"));
}
