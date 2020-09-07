/**
 * @file bs_tree.h
 * @brief Red-Black tree data structure.
 */
#pragma once
#include <vector>


#define to_pRBNode(x)    reinterpret_cast<RBNode*>(x)

/**
 * @brief Auxilary base class of a red-black tree.
 * @warning Don't use it directly.
 */
template <class _K, class _Cmp = std::less<_K>>
class RBTree {
protected:
   enum RBColor {
      RED,
      BLACK
   };
   struct RBNode {
      RBNode(const _K &k, void *l = nullptr, void *r = nullptr, void *par = nullptr) :
         key(k), color(RED), left(to_pRBNode(l)), right(to_pRBNode(r)), p(to_pRBNode(par)) {}
      _K key;
      RBColor color;
      RBNode *left, *right, *p;
   };
   RBNode _rb_null;
   RBNode* root = nullptr;
   void *RB_NULL;
   _Cmp cmp;
   size_t size = 0;

   virtual void RotateL(RBNode* x) {
      auto y = x->right;
      x->right = y->left;
      if (y->left != RB_NULL)
         y->left->p = x;
      y->p = x->p;
      if (x->p == RB_NULL)
         root = y;
      else if (x == x->p->left)
         x->p->left = y;
      else
         x->p->right = y;
      y->left = x;
      x->p = y;
   }

   virtual void RotateR(RBNode* x) {
      auto y = x->left;
      x->left = y->right;
      if (y->right != RB_NULL)
         y->right->p = x;
      y->p = x->p;
      if (x->p == RB_NULL)
         root = y;
      else if (x == x->p->left)
         x->p->left = y;
      else
         x->p->right = y;
      y->right = x;
      x->p = y;
   }

   void InsertFuxup(RBNode* z) {
      while (z->p->color == RED) {
         if (z->p == z->p->p->left) {
            auto y = z->p->p->right;
            if (y->color == RED) {
               z->p->color = BLACK;
               y->color = BLACK;
               z->p->p->color = RED;
               z = z->p->p;
            }
            else {
               if (z == z->p->right) {
                  z = z->p;
                  RotateL(z);
               }
               z->p->color = BLACK;
               z->p->p->color = RED;
               RotateR(z->p->p);
            }
         }
         else {
            auto y = z->p->p->left;
            if (y->color == RED) {
               z->p->color = BLACK;
               y->color = BLACK;
               z->p->p->color = RED;
               z = z->p->p;
            }
            else {
               if (z == z->p->left) {
                  z = z->p;
                  RotateR(z);
               }
               z->p->color = BLACK;
               z->p->p->color = RED;
               RotateL(z->p->p);
            }
         }
      }
      root->color = BLACK;
   }

   void DeleteFixup(RBNode* x) {
      while (x != root && x->color == BLACK) {
         if (x == x->p->left) {
            auto w = x->p->right;
            if (w->color == RED) {
               x->p->color = RED;
               w->color = BLACK;
               RotateL(x->p);
               w = x->p->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
               w->color = RED;
               x = x->p;
            }
            else {
               if (w->right->color == BLACK) {
                  w->left->color = BLACK;
                  w->color = RED;
                  RotateR(w);
                  w = x->p->right;
               }
               w->color = x->p->color;
               w->p->color = BLACK;
               w->right->color = BLACK;
               RotateL(x->p);
               x = root;
            }
         }
         else {
            auto w = x->p->left;
            if (w->color == RED) {
               x->p->color = RED;
               w->color = BLACK;
               RotateR(x->p);
               w = x->p->left;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
               w->color = RED;
               x = x->p;
            }
            else {
               if (w->left->color == BLACK) {
                  w->right->color = BLACK;
                  w->color = RED;
                  RotateL(w);
                  w = x->p->left;
               }
               w->color = x->p->color;
               w->p->color = BLACK;
               w->left->color = BLACK;
               RotateR(x->p);
               x = root;
            }
         }
      }
      x->color = BLACK;
   }

   void Transplant(RBNode* u, RBNode* v) {
      if (u->p == RB_NULL)
         root = v;
      else if (u == u->p->left)
         u->p->left = v;
      else
         u->p->right = v;
      v->p = u->p;
   }

   virtual void Delete(RBNode* z) {
      auto orig_color = z->color;
      RBNode* x = nullptr;
      if (z->left == RB_NULL) {
         x = z->right;
         Transplant(z, x);
      }
      else if (z->right == RB_NULL) {
         x = z->left;
         Transplant(z, x);
      }
      else {
         auto y = z->right;
         while (y->left != RB_NULL) y = y->left;
         orig_color = y->color;
         x = y->right;
         if (y->p == z)
            x->p = y;
         else {
            Transplant(y, x);
            y->right = z->right;
            y->right->p = y;
         }
         Transplant(z, y);
         y->left = z->left;
         y->left->p = y;
         y->color = z->color;
      }
      delete z;
      if (orig_color == BLACK)
         DeleteFixup(x);
   }

   void DestroyNodes(RBNode* x) {
      if (x == RB_NULL) return;
      DestroyNodes(x->left);
      DestroyNodes(x->right);
      delete x;
   }

   int HelthCheckDfs(RBNode* x, size_t &cnt) {
      if (x == RB_NULL) return 0;
      cnt++;
      int l = HelthCheckDfs(x->left, cnt);
      int r = HelthCheckDfs(x->right, cnt);
      if (l < 0 || l != r) return -1;
      if (x->color == RED && x->left && 
         (x->left->color != BLACK || x->right->color != BLACK)) return -1;
      return l + (x->color == BLACK);
   }

public:
   RBTree() :
      _rb_null(_K()), root(&_rb_null), RB_NULL(&_rb_null) {
      _rb_null.color = BLACK;
   }
   explicit RBTree(const _Cmp& c) :
      cmp(c), _rb_null(_K(), _V()), root(&_rb_null), RB_NULL(&_rb_null) {
      _rb_null.color = BLACK;
   }
   // don't need virtual destructor
   ~RBTree() {
      DestroyNodes(root);
   }

#if 0
   RBNode* Find(const _K& key) const {
      auto x = root;
      while (x != RB_NULL && x->key != key)
         x = cmp(x->key, key) ? x->right : x->left;
      return x == RB_NULL ? nullptr : x;
   }
#endif

   virtual size_t Remove(const _K& key) {
      auto x = root;
      while (x != RB_NULL && x->key != key)
         x = cmp(x->key, key) ? x->right : x->left;
      if (x == RB_NULL) return 0;
      Delete(x);
      size--;
      return 1;
   }

   virtual size_t Count(const _K& key) const {
      auto x = root;
      while (x != RB_NULL && x->key != key)
         x = cmp(x->key, key) ? x->right : x->left;
      return (x == RB_NULL) ? 0 : 1;
   }

   size_t Size() const { return size; }

   void Clear() {
      DestroyNodes(root);
      root = reinterpret_cast<RBNode*>(RB_NULL);
      size = 0;
   }

   virtual bool HealthCheck() {
      size_t cnt = 0;
      return HelthCheckDfs(root, cnt) >= 0 && cnt == size;
   }
};


/**
 * @brief Set based on a red-black tree.
 */
template <class _K, class _Cmp = std::less<_K>>
class RBTreeSet : public RBTree<_K, _Cmp> {
public:
   bool Insert(const _K& key) {
      auto p = RB_NULL;
      auto x = &root;
      while (*x != RB_NULL && (*x)->key != key) {
         p = *x;
         x = cmp((*x)->key, key) ? &(*x)->right : &(*x)->left;
      }
      if (*x != RB_NULL) return false;
      *x = new RBNode(key, RB_NULL, RB_NULL, p);
      size++;
      InsertFuxup(*x);
      return true;
   }
};


/**
 * @brief Map based on a red-black tree.
 */
template <class _K, class _V, class _Cmp = std::less<_K>>
class RBTreeMap : public RBTree<_K, _Cmp> {
protected:
   struct RBNodeMap : public RBNode {
      RBNodeMap(const _K &k, const _V &v, void *l = nullptr, void *r = nullptr, void *par = nullptr) :
         RBNode(k, l, r, par), val(v) {}
      _V val;
   };

   std::pair<RBNodeMap*, bool> Insert_(const _K& key, const _V& val) {
      auto p = RB_NULL;
      auto x = &root;
      while (*x != RB_NULL && (*x)->key != key) {
         p = *x;
         x = cmp((*x)->key, key) ? &(*x)->right : &(*x)->left;
      }
      if (*x != RB_NULL) return std::make_pair(reinterpret_cast<RBNodeMap*>(*x), false);
      auto res = new RBNodeMap(key, val, RB_NULL, RB_NULL, p);
      *x = reinterpret_cast<RBNode*>(res);
      size++;
      InsertFuxup(*x);
      return std::make_pair(res, true);
   }

public:
   _V& operator [] (const _K& key) {
      return Insert_(key, _V()).first->val;
   }
};


/**
 * @brief Auxilary base class for multi set and map.
 * @warning Don't use it directly.
 */
template <class _K, class _Cmp = std::less<_K>>
class RBTreeMulti : public RBTree<_K, _Cmp> {
protected:
   struct RBNodeMulti : public RBNode {
      RBNodeMulti(const _K &k, void *l = nullptr, void *r = nullptr, void *par = nullptr) :
         RBNode(k, l, r, par), cnt(1) {}
      size_t cnt;
   };

public:
   size_t Remove(const _K& key) override {
      auto x = root;
      while (x != RB_NULL && x->key != key)
         x = cmp(x->key, key) ? x->right : x->left;
      if (x == RB_NULL) return 0;
      auto cnt = reinterpret_cast<RBNodeMulti*>(x)->cnt;
      Delete(x);
      size -= cnt;
      return cnt;
   }

   size_t Count(const _K& key) const override {
      auto x = root;
      while (x != RB_NULL && x->key != key)
         x = cmp(x->key, key) ? x->right : x->left;
      return (x == RB_NULL) ? 0 : reinterpret_cast<RBNodeMulti*>(x)->cnt;
   }
};


/**
 * @brief Multi Set based on a red-black tree.
 */
template <class _K, class _Cmp = std::less<_K>>
class RBTreeMultiSet : public RBTreeMulti<_K, _Cmp> {
public:
   virtual void Insert(const _K& key) {
      auto p = RB_NULL;
      auto x = &root;
      while (*x != RB_NULL && (*x)->key != key) {
         p = *x;
         x = cmp((*x)->key, key) ? &(*x)->right : &(*x)->left;
      }
      size++;
      if (*x != RB_NULL) {
         reinterpret_cast<RBNodeMulti*>(*x)->cnt++;
      }
      else {
         *x = new RBNodeMulti(key, RB_NULL, RB_NULL, p);
         InsertFuxup(*x);
      }
   }
};


/**
 * @brief Multi Map based on a red-black tree.
 */
template <class _K, class _V, class _Cmp = std::less<_K>>
class RBTreeMultiMap : public RBTreeMulti<_K, _Cmp> {
protected:
   struct RBNodeMultiMap : public RBNodeMulti {
      RBNodeMultiMap(const _K &k, const _V &v, void *l = nullptr, void *r = nullptr, void *par = nullptr) :
         RBNodeMulti(k, l, r, par), vals(1, v) {}
      std::vector<_V> vals;
   };

public:
   virtual void Insert(const _K& key, const _V& val) {
      auto p = RB_NULL;
      auto x = &root;
      while (*x != RB_NULL && (*x)->key != key) {
         p = *x;
         x = cmp((*x)->key, key) ? &(*x)->right : &(*x)->left;
      }
      size++;
      if (*x != RB_NULL) {
         reinterpret_cast<RBNodeMultiMap*>(*x)->vals.emplace_back(val);
         reinterpret_cast<RBNodeMultiMap*>(*x)->cnt++;
      }
      else {
         *x = new RBNodeMultiMap(key, val, RB_NULL, RB_NULL, p);
         InsertFuxup(*x);
      }
   }

   std::vector<_V> const& Get(const _K& key) const {
      static const std::vector<_V> empty;
      auto x = root;
      while (x != RB_NULL && x->key != key)
         x = cmp(x->key, key) ? x->right : x->left;
      return (x == RB_NULL) ? empty : reinterpret_cast<RBNodeMultiMap*>(x)->vals;
   }
};


#define to_pRBONode(x)    reinterpret_cast<RBONode*>(x)

/**
 * @brief Base class of red-black tree with dynamic ordering capabilities.
 * @warning Don't use it directly.
 */
template <class _K, class _Cmp = std::less<_K>>
class RBTreeDynOrder : public RBTreeMulti<_K, _Cmp> {
protected:
   struct RBONode : RBNodeMulti {
      RBONode(const _K &k, void *l = nullptr, void *r = nullptr, void *par = nullptr) :
         RBNodeMulti(k, l, r, par), size(1) {}
      size_t size;
   };
   RBONode _rbo_null;

   void RotateL(RBNode* x) override {
      auto y = x->right;
      x->right = y->left;
      if (y->left != RB_NULL)
         y->left->p = x;
      y->p = x->p;
      if (x->p == RB_NULL)
         root = reinterpret_cast<RBNode*>(y);
      else if (x == x->p->left)
         x->p->left = y;
      else
         x->p->right = y;
      y->left = x;
      x->p = y;
      to_pRBONode(x)->size = to_pRBONode(x->left)->size + to_pRBONode(x->right)->size + to_pRBONode(x)->cnt;
      to_pRBONode(y)->size = to_pRBONode(x)->size + to_pRBONode(y->right)->size + to_pRBONode(y)->cnt;
   }

   void RotateR(RBNode* x) override {
      auto y = x->left;
      x->left = y->right;
      if (y->right != RB_NULL)
         y->right->p = x;
      y->p = x->p;
      if (x->p == RB_NULL)
         root = reinterpret_cast<RBNode*>(y);
      else if (x == x->p->left)
         x->p->left = y;
      else
         x->p->right = y;
      y->right = x;
      x->p = y;
      to_pRBONode(x)->size = to_pRBONode(x->left)->size + to_pRBONode(x->right)->size + to_pRBONode(x)->cnt;
      to_pRBONode(y)->size = to_pRBONode(x)->size + to_pRBONode(y->left)->size + to_pRBONode(y)->cnt;
   }

   void Delete(RBNode* z) override {
      auto orig_color = z->color;
      RBNode *x = nullptr;
      for (auto p = z->p; p != RB_NULL; p = p->p) to_pRBONode(p)->size--;
      if (z->left == RB_NULL) {
         x = z->right;
         Transplant(z, x);
      }
      else if (z->right == RB_NULL) {
         x = z->left;
         Transplant(z, x);
      }
      else {
         auto *y = z->right;
         while (y->left != RB_NULL) {
            to_pRBONode(y)->size--;
            y = y->left;
         }
         orig_color = y->color;
         x = y->right;
         if (y->p == z)
            x->p = y;
         else {
            Transplant(y, x);
            y->right = z->right;
            y->right->p = y;
         }
         Transplant(z, y);
         y->left = z->left;
         y->left->p = y;
         y->color = z->color;
         to_pRBONode(y)->size = to_pRBONode(y->left)->size + to_pRBONode(y->right)->size + to_pRBONode(y)->cnt;
      }
      delete z;
      if (orig_color == BLACK)
         DeleteFixup(x);
   }

   int HelthCheckDfs(RBONode* x, size_t &cnt) {
      if (x == RB_NULL) return 0;
      cnt++;
      int l = HelthCheckDfs(to_pRBONode(x->left), cnt);
      int r = HelthCheckDfs(to_pRBONode(x->right), cnt);
      if (l < 0 || l != r) return -1;
      if (to_pRBONode(x->left)->size + to_pRBONode(x->right)->size + x->cnt
         != x->size) return -1;
      if (x->color == RED && x->left && 
         (x->left->color != BLACK || x->right->color != BLACK)) return -1;
      return l + (x->color == BLACK);
   }

   const RBNode* GetNth(size_t n) {
      if (size <= n) return nullptr;
      auto x = root;
      while (true) {
         if (to_pRBONode(x->left)->size + to_pRBONode(x)->cnt > n &&
            to_pRBONode(x->left)->size <= n) return x;
         if (to_pRBONode(x->left)->size < n) {
            n -= to_pRBONode(x->left)->size + to_pRBONode(x)->cnt;
            x = x->right;
         }
         else {
            x = x->left;
         }
      }
   }

public:
   RBTreeDynOrder() :
      _rbo_null(_K()) {
      RB_NULL = root = to_pRBNode(&_rbo_null);
      _rbo_null.color = BLACK;
      _rbo_null.size = 0;
      _rbo_null.cnt = 0;
   }
   explicit RBTreeDynOrder(const _Cmp& c) :
      cmp(c), _rbo_null(_K()) {
      RB_NULL = root = to_pRBNode(&_rbo_null);
      _rbo_null.color = BLACK;
      _rbo_null.size = 0;
      _rbo_null.cnt = 0;
   }
   
   size_t GetPos(const _K& key, bool &exists) {
      auto x = root;
      size_t n = 0;
      while (x != RB_NULL && x->key != key) {
         if (cmp(x->key, key)) {
            n += to_pRBONode(x->left)->size + to_pRBONode(x)->cnt;
            x = x->right;
         }
         else {
            x = x->left;
         }
      }
      exists = x != RB_NULL;
      return exists ? n + to_pRBONode(x->left)->size : n + 1;
   }

   _K GetNthKey(size_t n, bool &succeeded) {
      auto x = GetNth(n);
      succeeded = x;
      return succeeded ? x->key : _K();
   }

   bool HealthCheck() override {
      size_t cnt = 0;
      return HelthCheckDfs(reinterpret_cast<RBONode*>(root), cnt) >= 0 && cnt == size;
   }
};


/**
 * @brief Dynamic ordering set based on a red-black tree.
 */
template <class _K, class _Cmp = std::less<_K>>
class DynOrderSet : public RBTreeDynOrder<_K, _Cmp> {
public:
   bool Insert(const _K& key) {
      auto p = to_pRBNode(RB_NULL);
      auto x = &root;
      while (*x != RB_NULL && (*x)->key != key) {
         p = *x;
         x = cmp((*x)->key, key) ? &(*x)->right : &(*x)->left;
      }
      if (*x != RB_NULL) return false;
      *x = new RBONode(key, RB_NULL, RB_NULL, p);
      size++;
      for (;p != RB_NULL; p = p->p) reinterpret_cast<RBONode*>(p)->size++;
      InsertFuxup(*x);
      return true;
   }
};


/**
 * @brief Dynamic ordering map based on a red-black tree.
 */
template <class _K, class _V, class _Cmp = std::less<_K>>
class DynOrderMap : public RBTreeDynOrder<_K, _Cmp> {
protected:
   struct RBONodeMap : public RBONode {
      RBONodeMap(const _K &k, const _V &v, void *l = nullptr, void *r = nullptr, void *par = nullptr) :
         RBONode(k, l, r, par), val(v) {}
      _V val;
   };

   std::pair<RBONodeMap*, bool> Insert_(const _K& key, const _V& val) {
      auto p = RB_NULL;
      auto x = &root;
      while (*x != RB_NULL && (*x)->key != key) {
         p = *x;
         x = cmp((*x)->key, key) ? &(*x)->right : &(*x)->left;
      }
      if (*x != RB_NULL) return std::make_pair(reinterpret_cast<RBNodeMap*>(*x), false);
      auto res = new RBONodeMap(key, val, RB_NULL, RB_NULL, p);
      *x = reinterpret_cast<RBNode*>(res);
      size++;
      InsertFuxup(*x);
      return std::make_pair(res, true);
   }

public:
   _V& operator [] (const _K& key) {
      return Insert_(key, _V()).first->val;
   }
};


/**
 * @brief Dynamic ordering multi set based on a red-black tree.
 */
template <class _K, class _Cmp = std::less<_K>>
class DynOrderMultiSet : public RBTreeDynOrder<_K, _Cmp> {
public:
   void Insert(const _K& key) {
      auto p = to_pRBNode(RB_NULL);
      auto x = &root;
      while (*x != RB_NULL && (*x)->key != key) {
         p = *x;
         x = cmp((*x)->key, key) ? &(*x)->right : &(*x)->left;
      }
      size++;
      for (;p != RB_NULL; p = p->p) reinterpret_cast<RBONode*>(p)->size++;
      if (*x != RB_NULL) {
         reinterpret_cast<RBONode*>(*x)->cnt++;
      }
      else {
         *x = new RBONode(key, RB_NULL, RB_NULL, p);
         InsertFuxup(*x);
      }
   }
};
