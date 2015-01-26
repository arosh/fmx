#pragma once

#include <cstdint>
#include <cassert>

#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <tuple>

#include "bitvector.h"
#include "bit_operation.h"

class WaveletMatrix {
public:
  typedef uint64_t Index;
private:
  Index n;
  const int log_sigma;
  std::vector<BitVector> BV;
  std::vector<Index> Z;

  Index down0(int depth, Index pos) const {
    assert(pos <= n);
    if (pos == n)
      return Z[depth];
    return BV[depth].rank0(pos);
  }

  Index down1(int depth, Index pos) const {
    assert(pos <= n);
    if (pos == n)
      return n;
    return Z[depth] + BV[depth].rank1(pos);
  }

  template <class T>
  std::pair<Index, Index> equal_range(const T c, const Index st,
                                      const Index en) const {
    assert(0 <= st && st <= n);
    assert(0 <= en && en <= n);
    assert(st <= en);
    Index L = st, R = en;
    for (int i = 0; i < log_sigma; ++i) {
      if (!bit_operation::get_bit(c, i)) {
        L = down0(i, L);
        R = down0(i, R);
      } else {
        L = down1(i, L);
        R = down1(i, R);
      }
    }
    return std::make_pair(L, R);
  }

public:
  WaveletMatrix(const int log_sigma_) : log_sigma(log_sigma_), Z(log_sigma_) {}

  std::vector<BitVector> BV_() { return BV; }
  Index length() const {
    return n;
  }

  template <class V> void init(const V &vec) {
    n = vec.size();
    BV.reserve(log_sigma);
    for(int d = 0; d < log_sigma; ++d) {
      BV.emplace_back(n);
    }
    Z.assign(log_sigma, 0ULL);

    const size_t alphaSize = size_t(1) << log_sigma;
    std::vector<size_t> offset(alphaSize, 0);
    for(int d = 0; d < log_sigma; ++d) {
      const size_t mask = ((size_t(1) << d) - 1);
      for (size_t i = 0; i < n; ++i) {
        if(bit_operation::get_bit(vec[i], d)) {
          BV[d].set(offset[vec[i] & mask]);
        }
        else {
          ++Z[d];
        }
        ++offset[vec[i] & mask];
      }
      BV[d].build();
      offset.assign(alphaSize, 0);
      const size_t mask2 = (size_t(1) << (d + 1)) - 1;
      for (size_t i = 0; i < n; ++i) {
        ++offset[vec[i] & mask2];
      }
      size_t cur = 0;
      for(size_t c = 0; c < alphaSize; ++c) {
        size_t tmp = offset[c];
        offset[c] = cur;
        cur += tmp;
      }
    }
  }

  template <class T> T access(const Index pos) const {
    assert(pos < n);
    T ret = 0;
    Index p = pos;
    for(int d = 0; d < log_sigma; ++d) {
      bool b = BV[d].get(p);
      if(!b) {
        p = BV[d].rank0(p);
      }
      else {
        ret |= (1ULL << d);
        p = Z[d] + BV[d].rank1(p);
      }
    }
    return ret;
  }

  template <class T> Index rank_lt(const T c) const {
    return equal_range(c, 0, n).first;
  }

  template <class T> Index rank(const T c, const Index pos) const {
    const auto eq_range = equal_range(c, 0, pos);
    return eq_range.second - eq_range.first;
  }

  template <class T> Index select(const T c, const Index i) const {
    assert(i < rank(c, n));
    Index p = rank_lt(c) + i;
    for(int d = log_sigma - 1; d >= 0; --d) {
      bool b = bit_operation::get_bit(c, d);
      if(!b) {
        if(p == n)
          p = Z[d] - 1;
        else
          p = BV[d].select0(p);
      }
      else {
        if(p == n)
          p = n - 1;
        else
          p = BV[d].select1(p - Z[d]);
      }
    }
    return p;
  }

  template <class T> struct RangeNode {
    T value;
    Index st, en;
    int depth;
    RangeNode() {}
    RangeNode(const T value_, const Index st_, const Index en_,
              const int depth_)
        : value(value_), st(st_), en(en_), depth(depth_) {}
    bool operator<(const RangeNode<T> &rhs) const {
      // std::tieがlvalueを要求する
      const Index lenL = en - st;
      const Index lenR = rhs.en - rhs.st;
      return std::tie(lenL, depth, value) <
             std::tie(lenR, rhs.depth, rhs.value);
    }
  };

  // vector<pair>をソートした時の結果のように返せると良い気がしたけど
  // 本当にそうでしょうか？
  template <class T>
  std::vector<std::pair<Index, T>> topk(const Index st, const Index en,
                                        const Index k) const {
    using namespace std;
    assert(st <= en);
    vector<pair<Index, T>> ret;
    if (st == en)
      return ret;
    priority_queue<RangeNode<T>> que;
    que.emplace(0, st, en, 0);
    while (que.empty() == false && ret.size() < k) {
      RangeNode<T> node = que.top();
      que.pop();
      assert(node.st < node.en);
      if (node.depth == log_sigma) {
        ret.emplace_back(node.en - node.st, node.value);
        continue;
      }
      RangeNode<T> l;
      l.value = node.value;
      l.st = down0(node.depth, node.st);
      l.en = down0(node.depth, node.en);
      l.depth = node.depth + 1;
      if (l.st < l.en)
        que.emplace(l);

      RangeNode<T> r;
      r.value = bit_operation::pop_bit(node.value, node.depth);
      r.st = down1(node.depth, node.st);
      r.en = down1(node.depth, node.en);
      r.depth = node.depth + 1;
      if (r.st < r.en)
        que.emplace(r);
    }
    return ret;
  }
}; // class WaveletMatrix
