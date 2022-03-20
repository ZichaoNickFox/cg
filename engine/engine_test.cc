#include "gtest/gtest.h"

#include "engine/util.h"

using namespace engine;

TEST(engine_test, util)
{
  EXPECT_EQ(util::GetFileExt("a.txt"), "txt");
  EXPECT_EQ(util::GetFileExt("a.fs.glsl"), "glsl");
  EXPECT_EQ(util::GetFileExt("a"), "");
}