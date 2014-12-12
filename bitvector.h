#pragma once

#include <cstdint>
#include <cassert>
#include <algorithm>
#include <vector>
#include <array>

namespace {
namespace bitvector_private {

typedef uint64_t Index;
Index ceil(Index a, Index b) { return (a + b - 1) / b; }

constexpr int kSizeS = 64;
constexpr int kSizeL = 4;

class BitVectorBacket {
  std::array<uint64_t, kSizeL> B;
  std::array<uint8_t, kSizeL> S;
#ifndef NDEBUG
  bool built;
#endif

public:
  BitVectorBacket() {
    B.fill(0ULL);
#ifndef NDEBUG
    built = false;
#endif
  }
  void set(Index i) {
#ifndef NDEBUG
    built = false;
    assert(i < kSizeS * kSizeL);
#endif
    const Index a = i / kSizeS;
    const Index b = i % kSizeS;
    B[a] |= 1ULL << b;
  }
  bool get(Index i) const {
#ifndef NDEBUG
    assert(i < kSizeS * kSizeL);
#endif
    const Index a = i / kSizeS;
    const Index b = i % kSizeS;
    return (B[a] >> b) & 1ULL;
  }
  Index build() {
#ifndef NDEBUG
    built = true;
#endif
    Index cur = 0;
    for (int i = 0; i < kSizeL; ++i) {
      S[i] = cur;
      cur += __builtin_popcountll(B[i]);
    }
    return cur;
  }
  uint8_t rank(Index i) const {
#ifndef NDEBUG
    assert(built);
    assert(i < kSizeS * kSizeL);
#endif
    const Index a = i / kSizeS;
    const Index b = i % kSizeS;
    const uint64_t mask = (1ULL << b) - 1;
    const uint8_t r = S[a] + __builtin_popcountll(B[a] & mask);
    return r;
  }
  Index select(uint8_t r) const {
#ifndef NDEBUG
    assert(built);
    assert(r < S[kSizeL - 1] + __builtin_popcountll(B[kSizeL - 1]));
#endif
    Index L, R;

    L = 0, R = kSizeL;
    while (R - L > 1) {
      Index M = (L + R) / 2;
      if (S[M] <= r) {
        L = M;
      } else {
        R = M;
      }
    }

    uint64_t BB = B[L];
    Index ret = kSizeS * L;

    r -= S[L];

    L = 0, R = kSizeS;
    while (R - L > 1) {
      Index M = (L + R) / 2;
      uint64_t mask = (1ULL << M) - 1;
      if (__builtin_popcountll(BB & mask) <= r) {
        L = M;
      } else {
        R = M;
      }
    }
    return ret + L;
  }
}; // class BitVectorBacket
}; // namespace bitvector_private

class BitVector {
  typedef bitvector_private::Index Index;
  static constexpr Index kSize =
      bitvector_private::kSizeS * bitvector_private::kSizeL;
  const Index n;
  std::vector<bitvector_private::BitVectorBacket> backets;
  std::vector<Index> S;
#ifndef NDEBUG
  bool built;
#endif

public:
  BitVector(Index n_)
      : n(n_), backets(bitvector_private::ceil(n_, kSize)),
        S(bitvector_private::ceil(n_, kSize)) {
#ifndef NDEBUG
    built = false;
#endif
  }
  Index build() {
#ifndef NDEBUG
    built = true;
#endif
    Index cur = 0;
    for (Index i = 0; i < backets.size(); ++i) {
      S[i] = cur;
      cur += backets[i].build();
    }
    return cur;
  }
  void set(Index i) {
#ifndef NDEBUG
    built = false;
    assert(i < n);
#endif
    Index a = i / kSize;
    Index b = i % kSize;
    backets[a].set(b);
  }
  bool get(Index i) const {
#ifndef NDEBUG
    assert(i < n);
#endif
    Index a = i / kSize;
    Index b = i % kSize;
    return backets[a].get(b);
  }
  Index rank(Index i) const {
#ifndef NDEBUG
    assert(built);
    assert(i < n);
#endif
    Index a = i / kSize;
    Index b = i % kSize;
    return S[a] + backets[a].rank(b);
  }
  Index select(Index r) const {
#ifndef NDEBUG
    assert(built);
    assert(r <=
           S[bitvector_private::ceil(n, kSize) - 1] +
               backets[bitvector_private::ceil(n, kSize) - 1].rank(kSize - 1));
#endif
    Index L, R;

    L = 0, R = bitvector_private::ceil(n, kSize);
    while (R - L > 1) {
      Index M = (L + R) / 2;
      if (S[M] <= r) {
        L = M;
      } else {
        R = M;
      }
    }

    const auto &backet = backets[L];
    Index ret = kSize * L;
    r -= S[L];
    return ret + backet.select(r);
  }
}; // class BitVector
} // namespace
