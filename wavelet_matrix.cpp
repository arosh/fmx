#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "wavelet_matrix.h"

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

TEST(WaveletMatrixTest, RankLessThanTest) {
  {
    vector<uint8_t> ary = { 2, 0, 0, 3, 1, 2, 2, 1 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ(0ULL, wm.rank_lt(0));
    EXPECT_EQ(2ULL, wm.rank_lt(1));
    EXPECT_EQ(4ULL, wm.rank_lt(2));
    EXPECT_EQ(7ULL, wm.rank_lt(3));
    EXPECT_EQ(8ULL, wm.rank_lt(4));
  }
  {
    vector<uint8_t> ary = { 3, 1, 4, 1, 5, 2, 2, 6 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ(0ULL, wm.rank_lt(0));
    EXPECT_EQ(0ULL, wm.rank_lt(1));
    EXPECT_EQ(2ULL, wm.rank_lt(2));
    EXPECT_EQ(4ULL, wm.rank_lt(3));
    EXPECT_EQ(5ULL, wm.rank_lt(4));
    EXPECT_EQ(6ULL, wm.rank_lt(5));
    EXPECT_EQ(7ULL, wm.rank_lt(6));
    EXPECT_EQ(8ULL, wm.rank_lt(7));
  }
}

TEST(WaveletMatrixTest, RankTest) {
  {
    vector<uint8_t> ary = { 2, 0, 0, 3, 1, 2, 2, 1 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ(2ULL, wm.rank(0, 8));
    EXPECT_EQ(2ULL, wm.rank(1, 8));
    EXPECT_EQ(3ULL, wm.rank(2, 8));
    EXPECT_EQ(1ULL, wm.rank(3, 8));
    EXPECT_EQ(0ULL, wm.rank(4, 8));
  }
  {
    vector<uint8_t> ary = { 3, 1, 4, 1, 5, 2, 2, 6 };
    WaveletMatrix wm(8);
    wm.init(ary);
    EXPECT_EQ(0ULL, wm.rank(1, 1));
    EXPECT_EQ(1ULL, wm.rank(1, 2));
    EXPECT_EQ(1ULL, wm.rank(1, 3));
    EXPECT_EQ(2ULL, wm.rank(1, 4));

    EXPECT_EQ(0ULL, wm.rank(2, 5));
    EXPECT_EQ(1ULL, wm.rank(2, 6));
    EXPECT_EQ(2ULL, wm.rank(2, 7));

    EXPECT_EQ(0ULL, wm.rank(3, 0));
    EXPECT_EQ(1ULL, wm.rank(3, 1));

    EXPECT_EQ(0ULL, wm.rank(4, 2));
    EXPECT_EQ(1ULL, wm.rank(4, 3));

    EXPECT_EQ(0ULL, wm.rank(5, 4));
    EXPECT_EQ(1ULL, wm.rank(5, 5));

    EXPECT_EQ(0ULL, wm.rank(6, 7));
  }
}
