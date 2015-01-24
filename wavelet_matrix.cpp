#include <vector>
#include <string>

#include "wavelet_matrix.h"

#ifdef __GNUC__
// https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html#Diagnostic-Pragmas
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#endif // __GNUC__

using namespace std;
typedef uint64_t Index;

string bitvector_to_string(const BitVector &bv, Index n) {
  string ret;
  for (Index i = 0; i < n; ++i) {
    ret.push_back(bv.get(i) ? '1' : '0');
  }
  return ret;
}

TEST(WaveletMatrixTest, InitTest) {
  {
    vector<uint8_t> ary = { 2, 0, 0, 3, 1, 2, 2, 1 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ("00011001", bitvector_to_string(wm.BV_()[0], 8));
    EXPECT_EQ("10011100", bitvector_to_string(wm.BV_()[1], 8));
  }
  {
    vector<uint8_t> ary = { 3, 1, 4, 1, 5, 2, 2, 6 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ("11011000", bitvector_to_string(wm.BV_()[0], 8));
    EXPECT_EQ("01111000", bitvector_to_string(wm.BV_()[1], 8));
    EXPECT_EQ("10010010", bitvector_to_string(wm.BV_()[2], 8));
  }
}

TEST(WaveletMatrixTest, AccessTest) {
  vector<uint8_t> ary = { 2, 0, 0, 3, 1, 2, 2, 1 };
  WaveletMatrix wm(8);
  wm.init(ary);
  EXPECT_EQ(2, wm.access<uint8_t>(0));
  EXPECT_EQ(0, wm.access<uint8_t>(1));
  EXPECT_EQ(0, wm.access<uint8_t>(2));
  EXPECT_EQ(3, wm.access<uint8_t>(3));
  EXPECT_EQ(1, wm.access<uint8_t>(4));
  EXPECT_EQ(2, wm.access<uint8_t>(5));
  EXPECT_EQ(2, wm.access<uint8_t>(6));
  EXPECT_EQ(1, wm.access<uint8_t>(7));
}

TEST(WaveletMatrixTest, RankLessThanTest) {
  {
    vector<uint8_t> ary = { 2, 0, 0, 3, 1, 2, 2, 1 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ(0, wm.rank_lt(0));
    EXPECT_EQ(2, wm.rank_lt(1));
    EXPECT_EQ(4, wm.rank_lt(2));
    EXPECT_EQ(7, wm.rank_lt(3));
    EXPECT_EQ(8, wm.rank_lt(4));
  }
  {
    vector<uint8_t> ary = { 3, 1, 4, 1, 5, 2, 2, 6 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ(0, wm.rank_lt(0));
    EXPECT_EQ(0, wm.rank_lt(1));
    EXPECT_EQ(2, wm.rank_lt(2));
    EXPECT_EQ(4, wm.rank_lt(3));
    EXPECT_EQ(5, wm.rank_lt(4));
    EXPECT_EQ(6, wm.rank_lt(5));
    EXPECT_EQ(7, wm.rank_lt(6));
    EXPECT_EQ(8, wm.rank_lt(7));
  }
}

TEST(WaveletMatrixTest, RankTest) {
  {
    vector<uint8_t> ary = { 2, 0, 0, 3, 1, 2, 2, 1 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ(2, wm.rank(0, 8));
    EXPECT_EQ(2, wm.rank(1, 8));
    EXPECT_EQ(3, wm.rank(2, 8));
    EXPECT_EQ(1, wm.rank(3, 8));
    EXPECT_EQ(0, wm.rank(4, 8));
  }
  {
    vector<uint8_t> ary = { 3, 1, 4, 1, 5, 2, 2, 6 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ(0, wm.rank(1, 1));
    EXPECT_EQ(1, wm.rank(1, 2));
    EXPECT_EQ(1, wm.rank(1, 3));
    EXPECT_EQ(2, wm.rank(1, 4));

    EXPECT_EQ(0, wm.rank(2, 5));
    EXPECT_EQ(1, wm.rank(2, 6));
    EXPECT_EQ(2, wm.rank(2, 7));

    EXPECT_EQ(0, wm.rank(3, 0));
    EXPECT_EQ(1, wm.rank(3, 1));

    EXPECT_EQ(0, wm.rank(4, 2));
    EXPECT_EQ(1, wm.rank(4, 3));

    EXPECT_EQ(0, wm.rank(5, 4));
    EXPECT_EQ(1, wm.rank(5, 5));

    EXPECT_EQ(0, wm.rank(6, 7));
  }
}

TEST(WaveletMatrixTest, SelectTest) {
  vector<uint8_t> ary = { 2, 0, 0, 3, 1, 2, 2, 1 };
  WaveletMatrix wm(8);
  wm.init(ary);
  EXPECT_EQ(0, wm.select(2, 0));
  EXPECT_EQ(1, wm.select(0, 0));
  EXPECT_EQ(2, wm.select(0, 1));
  EXPECT_EQ(3, wm.select(3, 0));
  EXPECT_EQ(4, wm.select(1, 0));
  EXPECT_EQ(5, wm.select(2, 1));
  EXPECT_EQ(6, wm.select(2, 2));
  EXPECT_EQ(7, wm.select(1, 1));
}

TEST(WaveletMatrixTest, TopKTest) {
  {
    vector<uint8_t> ary = { 2, 0, 0, 3, 1, 2, 2, 1 };
    WaveletMatrix wm(8);
    wm.init(ary);
    auto ret = wm.topk<uint8_t>(0, 8, 100);
    EXPECT_EQ(4, (int)ret.size());

    EXPECT_EQ(3, ret[0].first);
    EXPECT_EQ(2, ret[0].second);

    EXPECT_EQ(2, ret[1].first);
    EXPECT_EQ(1, ret[1].second);

    EXPECT_EQ(2, ret[2].first);
    EXPECT_EQ(0, ret[2].second);

    EXPECT_EQ(1, ret[3].first);
    EXPECT_EQ(3, ret[3].second);
  }

  {
    vector<uint8_t> ary = { 2, 0, 0, 3, 1, 2, 2, 1 };
    WaveletMatrix wm(8);
    wm.init(ary);
    auto ret = wm.topk<uint8_t>(0, 8, 2);
    EXPECT_EQ(2, (int)ret.size());

    EXPECT_EQ(3, ret[0].first);
    EXPECT_EQ(2, ret[0].second);

    EXPECT_EQ(2, ret[1].first);
    EXPECT_EQ(1, ret[1].second);
  }
}
