/**
 * @file segment_tree.h
 * @brief Fenwick and segment tree data structures.
 */
#pragma once


/**
 * @brief Fenwick tree data structure.
 * A Fenwick tree is a data structure that can efficiently update elements and
 * calculate prefix sums in a table of numbers.
 * It is a light-weight version of segment tree.
 * @see https://en.wikipedia.org/wiki/Fenwick_tree
 *
 * @tparam T Fenwick tree element type.
 */
template <class T>
class FenwickTree {
public:
   explicit FenwickTree(size_t n) :
         m_sum_(n + 1, 0) {}

   explicit FenwickTree(std::vector<T> arr) :
         m_sum_(arr.size() + 1, 0) {
      for (size_t i = 0; i < arr.size(); ++i) {
         Update(i, arr[i]);
      }
   }
#if 0 // Accumulate value in lower indexes
   /**
    * Add value to an element at specific position.
    */
   void Update(size_t i, T value) {
      for (i++; i != 0; i = prev(i))
         m_sum_[i] += value;
   }
   /**
    * Find sum of elements on segment [l, r].
    */
   T Query(size_t l, size_t r) const {
      T S = 0;
      for (auto i = l + 1; i < m_sum_.size(); i = next(i))
         S += m_sum_[i];
      for (auto i = r + 2; i < m_sum_.size(); i = next(i))
         S -= m_sum_[i];
      return S;
   }
#else // Accumulate value in greater indexes
   /**
    * Add value to an element at specific position.
    */
   void Update(size_t i, T value) {
      for (i++; i < m_sum_.size(); i = next(i))
         m_sum_[i] += value;
   }
   /**
    * Find sum of elements on segment [l, r].
    */
   T Query(size_t l, size_t r) const {
      T S = 0;
      for (auto i = r + 1; i != 0; i = prev(i))
         S += m_sum_[i];
      for (auto i = l; i != 0; i = prev(i))
         S -= m_sum_[i];
      return S;
   }
#endif

private:
#if 1 // 2 equivalent variants of getting next and previous array index.
   inline static size_t prev(size_t x) {
      return x - (x & (-x));
   }
   inline static size_t next(size_t x) {
      return x + (x & (-x));
   }
#else
   // on some architectures is faster
   inline static size_t prev(size_t x) {
      return x & (x - 1UL);
   }
   inline static size_t next(size_t x) {
      return (x << 1) - prev(x);
   }
#endif

   std::vector<T> m_sum_;
};


/**
 * @brief Segment tree data structure.
 * This data structure is a more generic version of Fenwick Tree.
 * It can apply any associative function on a range of elements. (See some
 * examples at the bottom of this file.) Fenwick tree supports only limited
 * set of associative functions. It does not support min/max function.
 * @see https://en.wikipedia.org/wiki/Segment_tree
 *
 * @tparam Func Associative function/operator (a ⊕ b) ⊕ c = a ⊕ (b ⊕ c).
 * @tparam T element type.
 */
template <class Func,
   class T = typename std::remove_const<decltype(Func::neutral)>::type>
class SegmentTree {
public:
   /**
    * @param n Number of elements in the array.
    * @param arr If provided, will initialize the trie with arr content, if nullptr will initialize with neutral element.
    */
   explicit SegmentTree(size_t n, const T* arr = nullptr) : n_(n) {
      tree.resize(n_ * 2, Func::neutral);
      if (arr != nullptr) {
         memcpy(tree.data() + n_, arr, n_ * sizeof(T));
         for (size_t i = n_ - 1; i != 0; i--) {
            tree[i] = Func::Operator(tree[i * 2], tree[i * 2 + 1]);
         }
      }
   }

   /**
    * Sets given value to the specific element of the array.
    * @param i Array index.
    * @param val Value to be set.
    */
   virtual void SetElem(size_t i, T val) {
      i += n_;
      tree[i] = val;
      for (i /= 2; i != 0; i /= 2) {
         tree[i] = Func::Operator(tree[i * 2], tree[i * 2 + 1]);
      }
   }

   virtual T Query(size_t l, size_t r) {
      l += n_; r += n_;
      T res = Func::neutral;
      while (l <= r) {
         if (l & 1) {
            res = Func::Operator(res, tree[l]);
            l++;
         }
         if ((r & 1) == 0) {
            res = Func::Operator(res, tree[r]);
            r--;
         }
         l /= 2;
         r /= 2;
      }
      return res;
   }

   virtual std::vector<T> GetArray() {
      return std::vector<T>(tree.data() + n_, tree.data() + n_ * 2);
   }

protected:
   size_t n_;
   std::vector<T> tree;
};

/**
 * @brief Segment tree data structure with ranges lazy update functionality.
 * This segment tree can efficiently update a range of values in one operation.
 *
 * @tparam Func Associative function/operator (a ⊕ b) ⊕ c = a ⊕ (b ⊕ c).
 * @tparam T element type.
 */
template <class Func, // Associative function/operator (a ⊕ b) ⊕ c = a ⊕ (b ⊕ c).
   class T = typename std::remove_const<decltype(Func::neutral)>::type>
class LazySegmentTree final : public SegmentTree<Func> {
public:
   /**
    * @param n Number of elements in the array.
    * @param arr If provided, will initialize the trie with arr content, if nullptr will initialize with neutral element.
    */
   explicit LazySegmentTree(size_t n, const T* arr = nullptr) : SegmentTree<Func>(n, arr), h_(1) {
      lazy.resize(SegmentTree<Func>::n_, Func::neutral);
      while (1 << h_ < SegmentTree<Func>::n_) {
         h_++;
      }
   }

   void SetElem(size_t i, T val) override {
      _Push(i + SegmentTree<Func>::n_);
      SegmentTree<Func>::SetElem(i, val);
   }

   T Query(size_t l, size_t r) override {
      _Push(l + SegmentTree<Func>::n_);
      _Push(r + SegmentTree<Func>::n_);
      return SegmentTree<Func>::Query(l, r);
   }

   /**
    * Update a range [l, r] of elements in the array by applying Func::RangeUpdate
    * @param l Left index.
    * @param r Right index.
    * @param val Update value.
    */
   void RangeUpdate(size_t l, size_t r, T val) {
      l += SegmentTree<Func>::n_;
      r += SegmentTree<Func>::n_;
      auto l0 = l / 2, r0 = r / 2;
      while (l <= r) {
         if (l & 1) {
            _Apply(l, val);
            l++;
         }
         if ((r & 1) == 0) {
            _Apply(r, val);
            r--;
         }
         l /= 2;
         r /= 2;
      }
      _Pull(l0);
      _Pull(r0);
   }

   std::vector<T> GetArray() override {
      _PushAll();
      return SegmentTree<Func>::GetArray();
   }

protected:
   /**
    * Pull updated value from the bottom to the top.
    * @param x Tree node index.
    */
   void _Pull(size_t x) {
      for (; x != 0; x >>= 1) {
         SegmentTree<Func>::tree[x] =
            Func::Operator(SegmentTree<Func>::tree[x * 2], SegmentTree<Func>::tree[x * 2 + 1]);
         Func::RangeUpdate(SegmentTree<Func>::tree[x], lazy[x], x, SegmentTree<Func>::n_);
      }
   }

   /**
    * Push lazy values dipper by sequentially applying values from parents to children.
    * @param x Tree node index.
    */
   void _Push(size_t x) {
      for (size_t h = h_; h != 0; --h) {
         size_t y = x >> h;
         if (lazy[y] != Func::neutral) {
            _Apply(y * 2, lazy[y]);
            _Apply(y * 2 + 1, lazy[y]);
            lazy[y] = Func::neutral;
         }
      }
   }

   void _PushAll() {
      for (size_t y = 1; y < SegmentTree<Func>::n_; ++y) {
         if (lazy[y] != Func::neutral) {
            _Apply(y * 2, lazy[y]);
            _Apply(y * 2 + 1, lazy[y]);
            lazy[y] = Func::neutral;
         }
      }
   }

   /**
    * Applies value of RangeUpdate to all leafs of given node.
    * Not directly, but by using lazy propagation.
    * @param x Tree node index.
    * @param val Value form update function.
    */
   void _Apply(size_t x, T val) {
      Func::RangeUpdate(SegmentTree<Func>::tree[x], val,
                        x, SegmentTree<Func>::n_);
      if (x < SegmentTree<Func>::n_) {
         Func::LazyUpdate(lazy[x], val);
      }
   }

   size_t h_; // height of the tree
   std::vector<T> lazy; // stores values for 'lazy' range update (value is equivalent to single element update value)
};


/**
 * Calculates number of leafs connected to given node.
 * This is required for lazy update of a range of elements (add some value to every element).
 * @param x Tree node index.
 * @param n Total elements in the array.
 */
size_t GetLeafsNum(size_t x, size_t n) {
   size_t leafs_num = 1;
   for (; x < n; x *= 2) {
      leafs_num *= 2;
   }

   size_t dif = (leafs_num + x) / 2;
   if (dif > n) {
      leafs_num -= dif - n;
   }

   return leafs_num;
}


template <class T>
class SumFunc {
public:
   /**
    * Associative function/operator (a ⊕ b) ⊕ c = a ⊕ (b ⊕ c).
    */
   inline static T Operator(T a, T b) {
      return a + b;
   }
   /**
    * Applies arr[i] += val to all i in range.
    */
   inline static void RangeUpdate(T& target, T update_val, size_t x, size_t n) {
      target += update_val * GetLeafsNum(x, n);
   }
   inline static void LazyUpdate(T& target, T update_val) {
      target += update_val;
   }
   static constexpr T neutral = 0; // neutral element a ⊕ a = a.
};

template <class T>
class MinFunc {
public:
   /**
    * Associative function/operator (a ⊕ b) ⊕ c = a ⊕ (b ⊕ c).
    */
   inline static T Operator(T a, T b) {
      return a < b ? a : b;
   }
   /**
    * Applies arr[i] = min(arr[i], val) to all i in range.
    */
   inline static void RangeUpdate(T& target, T val, size_t x, size_t n) {
      target = Operator(target, val);
   }
   inline static void LazyUpdate(T& target, T update_val) {
      target = Operator(target, update_val);
   }
   static constexpr T neutral = INT_MAX; // neutral element a ⊕ a = a.
};

template <class T>
class MaxFunc {
public:
   /**
    * Associative function/operator (a ⊕ b) ⊕ c = a ⊕ (b ⊕ c).
    */
   inline static T Operator(T a, T b) {
      return a > b ? a : b;
   }
   /**
    * Applies arr[i] = max(arr[i], val) to all i in range.
    */
   inline static void RangeUpdate(T& target, T update_val, size_t x, size_t n) {
      target = Operator(target, update_val);
   }
   inline static void LazyUpdate(T& target, T update_val) {
      target = Operator(target, update_val);
   }
   static constexpr T neutral = INT_MIN; // neutral element a ⊕ a = a.
};
