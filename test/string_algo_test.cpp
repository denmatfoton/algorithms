/**
 * @file sorting_test.cpp
 * @brief Tests for radix sort.
 */
#include <random>
#include <chrono>
#include <gtest/gtest.h>

#include "string_algo.h"

using namespace std;

/**
 * Longest duplicate substring search.
 * Given a string S, consider all duplicated substrings: (contiguous) substrings of S
 * that occur 2 or more times.  The occurrences may overlap.
 * Return any duplicated substring that has the longest possible length.
 * If S does not have a duplicated substring, the answer is "".
 *
 * @note This is a problem #1044 from http://leetcode.com.
 */
string longestDupSubstring(string &S, vector<size_t> (*sa_func)(const char*, size_t)) {
   auto n = S.size();
   auto sa = sa_func(S.c_str(), n);
   auto lcp = LcpSakai(S.c_str(), n, sa);
   auto i = max_element(lcp.begin(), lcp.end()) - lcp.begin();
   return S.substr(sa[i], lcp[i]);
}

/**
 * String similarity problem from Hackerrank.
 * @see https://www.hackerrank.com/challenges/string-similarity/problem
 */
int64_t stringSimilarity(std::string& s) {
   auto n = s.size();
   auto sa = ComputeSa(s.c_str(), n);
   auto lcp = LcpSakai(s.c_str(), n, sa);

   int64_t sum = n;
   size_t i = 0;
   for (;; ++i) if (sa[i] == 0) break;

   size_t m = i + 1 < n ? lcp[i + 1] : 0;
   for (size_t j = i; ++j < n;) {
      if (lcp[j] == 0) break;
      sum += m = std::min(m, lcp[j]);
   }
   m = lcp[i];
   for (size_t j = i; j; --j) {
      if (lcp[j] == 0) break;
      sum += m = std::min(m, lcp[j]);
   }

   return sum;
}


TEST(String, SuffixArray) {
   string s = "banana";
   auto res = longestDupSubstring(s, ComputeSaIS<char>);
   EXPECT_TRUE(res == "ana");
   res = longestDupSubstring(s, ComputeSa<char>);
   EXPECT_TRUE(res == "ana");

   s = "moplvidmaagmsiyyrkchbyhivlqwqsjcgtumqscmxrxrvwsnjjvygrelcbjgbpounhuyealllginkitfaiviraqcyc"
      "jmskrozcdqylbuejrgfnquercvghppljmojfvylcxakyjxnampmakyjbqgwbyokaybcuklkaqzawageypfqhhasetug"
      "atdaxpvtevrigynxbqodiyioapgxqkndujeranxgebnpgsukybyowbxhgpkwjfdywfkpufcxzzqiuglkakibbkobonu"
      "nnzwbjktykebfcbobxdflnyzngheatpcvnhdwkkhnlwnjdnrmjaevqopvinnzgacjkbhvsdsvuuwwhwesgtdzuctshy"
      "tyfugdqswvxisyxcxoihfgzxnidnfadphwumtgdfmhjkaryjxvfquucltmuoosamjwqqzeleaiplwcbbxjxxvgsnono"
      "ivbnmiwbnijkzgoenohqncjqnckxbhpvreasdyvffrolobxzrmrbvwkpdbfvbwwyibydhndmpvqyfmqjwosclwxhgxm"
      "wjiksjvsnwupraojuatksjfqkvvfroqxsraskbdbgtppjrnzpfzabmcczlwynwomebvrihxugvjmtrkzdwuafozjcfq"
      "acenabmmxzcueyqwvbtslhjeiopgbrbvfbnpmvlnyexopoahgmwplwxnxqzhucdieyvbgtkfmdeocamzenecqlbhqmd"
      "frvpsqyxvkkyfrbyolzvcpcbkdprttijkzcrgciidavsmrczbollxbkytqjwbiupvsorvkorfriajdtsowenhpmdtva"
      "mkoqacwwlkqfdzorjtepwlemunyrghwlvjgaxbzawmikfhtaniwviqiaeinbsqidetfsdbgsydkxgwoqyztaqmyeefa"
      "ihmgrbxzyheoegawthcsyyrpyvnhysynoaikwtvmwathsomddhltxpeuxettpbeftmmyrqclnzwljlpxazrzzdosemw"
      "mthcvgwtxtinffopqxbufjwsvhqamxpydcnpekqhsovvqugqhbgweaiheeicmkdtxltkalexbeftuxvwnxmqqjeyour"
      "vbdfikqnzdipmmmiltjapovlhkpunxljeutwhenrxyfeufmzipqvergdkwptkilwzdxlydxbjoxjzxwcfmznfqgoaem"
      "rrxuwpfkftwejubxkgjlizljoynvidqwxnvhngqakmmehtvykbjwrrrjvwnrteeoxmtygiiygynedvfzwkvmffghudu"
      "spyyrnftyvsvjstfohwwyxhmlfmwguxxzgwdzwlnnltpjvnzswhmbzgdwzhvbgkiddhirgljbflgvyksxgnsvztcywp"
      "vutqryzdeerlildbzmtsgnebvsjetdnfgikrbsktbrdamfccvcptfaaklmcaqmglneebpdxkvcwwpndrjqnpqgbgihs"
      "feotgggkdbvcdwfjanvafvxsvvhzyncwlmqqsmledzfnxxfyvcmhtjreykqlrfiqlsqzraqgtmocijejneeezqxbtom"
      "kwugapwesrinfiaxwxradnuvbyssqkznwwpsbgatlsxfhpcidfgzrc";

   res = longestDupSubstring(s, ComputeSaIS<char>);
   EXPECT_TRUE(res == "akyj");
   res = longestDupSubstring(s, ComputeSa<char>);
   EXPECT_TRUE(res == "akyj");
   
   auto start = chrono::high_resolution_clock::now();
   auto sa_is = ComputeSaIS(s.c_str(), s.size());
   auto end = chrono::high_resolution_clock::now();
   auto sa_is_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << "Suffix Array construction using Induced Sorting time elapsed: " << sa_is_time << "us\n";

   start = chrono::high_resolution_clock::now();
   auto sa = ComputeSa(s.c_str(), s.size());
   end = chrono::high_resolution_clock::now();
   auto sa_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << "Suffix Array construction using Radix Sort time elapsed: " << sa_time << "us\n";

   EXPECT_EQ(memcmp(sa_is.data(), sa.data(), sa_is.size() * sizeof(size_t)), 0);

   auto similarity = stringSimilarity(s);
   EXPECT_EQ(similarity, 1668);
   
   random_device rd;
   mt19937 gen(rd());
   uniform_int_distribution<int> dist('a', 'b');

   s.clear();
   for (int i = 0; i < 100'000; ++i) s.push_back(dist(gen));
   
   start = chrono::high_resolution_clock::now();
   sa_is = ComputeSaIS(s.c_str(), s.size());
   end = chrono::high_resolution_clock::now();
   sa_is_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << "Suffix Array construction using Induced Sorting time elapsed: " << sa_is_time << "us\n";

   start = chrono::high_resolution_clock::now();
   sa = ComputeSa(s.c_str(), s.size());
   end = chrono::high_resolution_clock::now();
   sa_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
   cout << "Suffix Array construction using Radix Sort time elapsed: " << sa_time << "us\n";

   EXPECT_EQ(memcmp(sa_is.data(), sa.data(), sa_is.size() * sizeof(size_t)), 0);
}
