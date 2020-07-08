/**
 * @file bs_tree_test.cpp
 * @brief Tests for Binary Search Trees.
 */
#include <algorithm>
#include <random>
#include <chrono>
#include <unordered_set>
#include <gtest/gtest.h>

#include "bs_tree.h"

using namespace std;


template <typename T>
void BsTreeGeneric(size_t elem_cnt, const char* name) {
   random_device rd;  //Will be used to obtain a seed for the random number engine
   mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
   uniform_int_distribution<int> dist(0, 1000000);

   T tree;
   set<int> dup;

   vector<int> data;
   for (size_t i = elem_cnt; i--;) data.push_back(dist(gen));

   auto start = chrono::high_resolution_clock::now();
   for (int k : data) tree.Insert(k);
   auto end = chrono::high_resolution_clock::now();
   auto rb_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << name << " insert time elapsed: " << rb_time << "us\n";
   
   start = chrono::high_resolution_clock::now();
   for (int k : data) dup.insert(k);
   end = chrono::high_resolution_clock::now();
   auto std_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << "Standard map insert time elapsed: " << std_time << "us\n";

   EXPECT_EQ(tree.Size(), dup.size());
   EXPECT_TRUE(tree.HealthCheck());

   shuffle(data.begin(), data.end(), gen);
   
   start = chrono::high_resolution_clock::now();
   for (size_t i = elem_cnt / 2; i--;) tree.Remove(data[i]);
   end = chrono::high_resolution_clock::now();
   rb_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << name << " remove time elapsed: " << rb_time << "us\n";

   start = chrono::high_resolution_clock::now();
   for (size_t i = elem_cnt / 2; i--;) dup.erase(data[i]);
   end = chrono::high_resolution_clock::now();
   std_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << "Standard map remove time elapsed: " << std_time << "us\n";
   
   EXPECT_EQ(tree.Size(), dup.size());
   EXPECT_TRUE(tree.HealthCheck());

   tree.Clear();
   dup.clear();
   
   sort(data.begin(), data.end());

   start = chrono::high_resolution_clock::now();
   for (int k : data) tree.Insert(k);
   end = chrono::high_resolution_clock::now();
   rb_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << name << " insert time elapsed: " << rb_time << "us\n";
   
   start = chrono::high_resolution_clock::now();
   for (int k : data) dup.insert(k);
   end = chrono::high_resolution_clock::now();
   std_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << "Standard map insert time elapsed: " << std_time << "us\n";

   EXPECT_EQ(tree.Size(), dup.size());
   EXPECT_TRUE(tree.HealthCheck());
}


TEST(BsTree, RedBlackGeneric) {
   BsTreeGeneric<RBTreeSet<int>>(100000, "Red-Black Tree");
}


TEST(BsTree, RBTreeDynOrderGeneric) {
   BsTreeGeneric<DynOrderSet<int>>(100000, "Red-Black Dynamic Ordering Tree");
}


TEST(BsTree, CheckDynOrder) {
   random_device rd;  //Will be used to obtain a seed for the random number engine
   mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
   uniform_int_distribution<int> dist(0, 10000);

   DynOrderSet<int, greater<int>> tree;
   unordered_set<int> s;
   constexpr size_t elem_cnt = 1000;
   for (size_t i = elem_cnt; i--;) s.insert(dist(gen));
   vector<int> data(s.begin(), s.end());
   
   for (int k : data) tree.Insert(k);

   sort(data.begin(), data.end(), greater<int>());
   uniform_int_distribution<size_t> idx(0, data.size() - 1);

   for (size_t i = elem_cnt / 2; i--;) {
      auto j = idx(gen);
      bool succeeded = false;
      auto key = tree.GetNthKey(j, succeeded);
      EXPECT_TRUE(succeeded);
      EXPECT_EQ(key, data[j]);
   }
   
   bool succeeded = false;
   auto key = tree.GetNthKey(data.size(), succeeded);
   EXPECT_FALSE(succeeded);

   for (size_t i = elem_cnt / 2; i--;) {
      auto j = idx(gen);
      key = data[j];
      bool exists = false;
      EXPECT_EQ(j, tree.GetPos(key, exists));
      EXPECT_TRUE(exists);
   }
   
   bool exists = false;
   EXPECT_EQ(data.size(), tree.GetPos(data.back() + 1, exists));
   EXPECT_FALSE(exists);
}
