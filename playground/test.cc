#include "gtest/gtest.h"

#include "playground/util.h"

TEST(engine_test, util)
{
  EXPECT_EQ(util::GetFileExt("a.txt"), "txt");
  EXPECT_EQ(util::GetFileExt("a.fs.glsl"), "glsl");
  EXPECT_EQ(util::GetFileExt("a"), "");

  for (int i = 0; i < 10000000; ++i) {
    float rand_value = util::RandFromTo(-10.0, 10.0);
    EXPECT_LE(rand_value, 10.0);
    EXPECT_GE(rand_value, -10.0);
  }
}