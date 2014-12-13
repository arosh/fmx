#include "bitvector.h"

#ifdef __GNUC__
// https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html#Diagnostic-Pragmas
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#endif // __GNUC__

TEST(BitVectorTest, ConstantTest) {
  EXPECT_EQ(64, bitvector_private::kSizeS);
  EXPECT_EQ(4, bitvector_private::kSizeL);
}

TEST(BitVectorBacketTest, RankTest) {
  using namespace bitvector_private;
  {
    BitVectorBacket bv;
    EXPECT_EQ(0, bv.build());
  }

  {
    BitVectorBacket bv;
    bv.set(0);
    EXPECT_EQ(1, bv.build());
    EXPECT_EQ(0, bv.rank(0));
    EXPECT_EQ(1, bv.rank(1));
  }

  {
    BitVectorBacket bv;
    bv.set(1);
    EXPECT_EQ(1, bv.build());
    EXPECT_EQ(0, bv.rank(0));
    EXPECT_EQ(0, bv.rank(1));
    EXPECT_EQ(1, bv.rank(2));
  }

  {
    BitVectorBacket bv;
    for (int i = 0; i < kSizeS * kSizeL; ++i) {
      bv.set(i);
    }
    EXPECT_EQ(static_cast<uint64_t>(kSizeS * kSizeL), bv.build());
    for (int i = 0; i < kSizeS * kSizeL; ++i) {
      EXPECT_EQ(i, bv.rank(i));
    }
  }
}

TEST(BitVectorBacketTest, Select1Test) {
  using namespace bitvector_private;
  {
    BitVectorBacket bv;
    bv.set(0);
    bv.build();
    EXPECT_EQ(0, bv.select1(0));
  }

  {
    BitVectorBacket bv;
    bv.set(1);
    bv.build();
    EXPECT_EQ(1, bv.select1(0));
  }

  {
    BitVectorBacket bv;
    bv.set(kSizeS * kSizeL - 1);
    EXPECT_EQ(1, bv.build());
    EXPECT_TRUE(bv.get(kSizeS * kSizeL - 1));
    EXPECT_EQ(static_cast<uint64_t>(kSizeS * kSizeL - 1), bv.select1(0));
  }
}

TEST(BitVectorBacketTest, Select0Test) {
  using namespace bitvector_private;
  {
    BitVectorBacket bv;
    bv.build();
    EXPECT_EQ(0, bv.select0(0));
  }

  {
    BitVectorBacket bv;
    bv.set(1);
    bv.build();
    EXPECT_EQ(2, bv.select0(1));
  }

  {
    BitVectorBacket bv;
    for (int i = 0; i < kSizeS * kSizeL - 1; ++i) {
      bv.set(i);
    }
    EXPECT_EQ(255, bv.build());
    EXPECT_FALSE(bv.get(kSizeS * kSizeL - 1));
    EXPECT_EQ(static_cast<uint64_t>(kSizeS * kSizeL - 1), bv.select0(0));
  }
}

TEST(BitVectorTest, Rank1Test) {
  {
    BitVector bv(0);
    EXPECT_EQ(0, bv.build());
  }

  {
    BitVector bv(2);
    bv.set(0);
    EXPECT_EQ(1, bv.build());
    EXPECT_EQ(0, bv.rank1(0));
    EXPECT_EQ(1, bv.rank1(1));
  }

  {
    BitVector bv(3);
    bv.set(1);
    EXPECT_EQ(1, bv.build());
    EXPECT_EQ(0, bv.rank1(0));
    EXPECT_EQ(0, bv.rank1(1));
    EXPECT_EQ(1, bv.rank1(2));
  }

  {
    BitVector bv(12345);
    for (int i = 0; i < 12345; ++i) {
      bv.set(i);
    }
    EXPECT_EQ(12345, bv.build());
    for (int i = 0; i < 12345; ++i) {
      EXPECT_EQ(static_cast<uint64_t>(i), bv.rank1(i));
    }
  }
}

TEST(BitVectorTest, Rank0Test) {
  {
    BitVector bv(2);
    EXPECT_EQ(0, bv.build());
    EXPECT_EQ(0, bv.rank0(0));
    EXPECT_EQ(1, bv.rank0(1));
  }

  {
    BitVector bv(3);
    bv.set(1);
    EXPECT_EQ(1, bv.build());
    EXPECT_EQ(0, bv.rank0(0));
    EXPECT_EQ(1, bv.rank0(1));
    EXPECT_EQ(1, bv.rank0(2));
  }
}

TEST(BitVector, Select1Test) {
  {
    BitVector bv(1);
    bv.set(0);
    bv.build();
    EXPECT_EQ(0, bv.select1(0));
  }

  {
    BitVector bv(2);
    bv.set(1);
    bv.build();
    EXPECT_EQ(1, bv.select1(0));
  }

  {
    BitVector bv(12345);
    bv.set(12345 - 1);
    EXPECT_EQ(1, bv.build());
    EXPECT_TRUE(bv.get(12345 - 1));
    EXPECT_EQ(static_cast<uint64_t>(12345 - 1), bv.select1(0));
  }

  {
    BitVector bv(12345);
    for (int i = 0; i < 12345; ++i) {
      bv.set(i);
    }
    EXPECT_EQ(12345, bv.build());
    for (int i = 0; i < 12345; ++i) {
      EXPECT_EQ(static_cast<uint64_t>(i), bv.select1(i));
    }
  }
}

TEST(BitVector, Select0Test) {
  {
    BitVector bv(1);
    bv.build();
    EXPECT_EQ(0, bv.select0(0));
  }

  {
    BitVector bv(2);
    bv.set(0);
    bv.build();
    EXPECT_EQ(1, bv.select0(0));
  }
}
