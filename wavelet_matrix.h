#pragma once

#include <cstdint>
#include <cassert>
#include <vector>

#include "bitvector.h"

namespace {
class WaveletMatrix {
  typedef uint64_t Index;
  Index n;
  const int log_sigma;
  std::vector<BitVector> BV;
  std::vector<Index> Z;

public:
  WaveletMatrix(const int log_sigma_) : log_sigma(log_sigma_), Z(log_sigma_) {}
  template <class V> void init(const V &vec) {
    n = vec.size();
    BV.reserve(log_sigma);
    std::fill(Z.begin(), Z.end(), 0ULL);
    for (int d = 0; d < log_sigma; ++d) {
      BV.emplace_back(n);
      for (Index i = 0; i < n; ++i) {
        if (!((vec[i] >> d) & 1ULL)) {
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
          if ((cur[i] >> d) & 1ULL) {
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
  template <class T> std::pair<Index, Index> equal_range(const T c, const Index st, const Index en) {
    Index L = st, R = en;
    for(int i = 0; i < log_sigma; ++i) {
      bool b = (c >> i) & 1ULL;
      if(!b) {
        if(L == en) {
          L = Z[i];
        }
        else {
          L = BV[i].rank0(L);
        }

        if(R == en) {
          R = Z[i];
        }
        else {
          R = BV[i].rank0(R);
        }
      }
      else {
        if(L == en) {
          L = n;
        }
        else {
          L = Z[i] + BV[i].rank1(L);
        }
        if(R == en) {
          R = n;
        }
        else {
          R = Z[i] + BV[i].rank1(R);
        }
      }
    }
    return std::make_pair(L, R);
  }
  template <class T> Index rank_lt(const T c) {
    return equal_range(c, 0ULL, n).first;
  }
  template <class T> Index rank(const T c, const Index pos) {
    const auto eq_range = equal_range(c, 0ULL, pos);
    return eq_range.second - eq_range.first;
  }

  std::vector<BitVector> BV_() { return BV; }
}; // class WaveletMatrix
}; // namespace
