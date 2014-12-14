#pragma once

#include <cstdint>
#include <cassert>

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
    if (pos == n)
      return Z[depth];
    return BV[depth].rank0(pos);
  }

  Index down1(int depth, Index pos) const {
    if (pos == n)
      return n;
    return Z[depth] + BV[depth].rank1(pos);
  }

  template <class T>
  std::pair<Index, Index> equal_range(const T c, const Index st,
                                      const Index en) const {
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
    std::fill(Z.begin(), Z.end(), 0ULL);
    for (int d = 0; d < log_sigma; ++d) {
      BV.emplace_back(n);
      for (Index i = 0; i < n; ++i) {
        if (!bit_operation::get_bit(vec[i], d)) {
          ++Z[d];
        }
      }
    }

    {
      V cur = vec;
      for (int d = 0; d < log_sigma; ++d) {
        V next(n);
        Index zero_pos = 0;
        Index one_pos = Z[d];

        for (Index i = 0; i < n; ++i) {
          if (bit_operation::get_bit(cur[i], d)) {
            BV[d].set(i);
            next[one_pos++] = cur[i];
          } else {
            next[zero_pos++] = cur[i];
          }
        }
        BV[d].build();
        cur = next;
      }
    }
  }

  template <class T> Index rank_lt(const T c) const {
    return equal_range(c, 0, n).first;
  }

  template <class T> Index rank(const T c, const Index pos) const {
    const auto eq_range = equal_range(c, 0, pos);
    return eq_range.second - eq_range.first;
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
