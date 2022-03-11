#include "gtest/gtest.h"

#include "engine/file_util.h"

using namespace engine;

TEST(engine_test, file_util)
{
  EXPECT_EQ(file_util::GetFileExt("a.txt"), "txt");
  EXPECT_EQ(file_util::GetFileExt("a.fs.glsl"), "glsl");
  EXPECT_EQ(file_util::GetFileExt("a"), "");
}