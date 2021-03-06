#include "gtest/gtest.h"

#include <glm/glm.hpp>
#include <map>
#include <math.h>

#include "engine/math.h"
#include "engine/util.h"

TEST(engine_test, util)
{
  EXPECT_EQ(util::FileExt("a.txt"), "txt");
  EXPECT_EQ(util::FileExt("a.fs.glsl"), "glsl");
  EXPECT_EQ(util::FileExt("a"), "");

  for (int i = 0; i < 10000000; ++i) {
    float rand_value = engine::RandFromTo(-10.0, 10.0);
    EXPECT_LE(rand_value, 10.0);
    EXPECT_GE(rand_value, -10.0);
  }

  EXPECT_EQ(util::FileName("a/target.png"), "target");
  EXPECT_EQ(util::FileName("target.png"), "target");
  EXPECT_EQ(util::FileName("target"), "target");
  EXPECT_EQ(util::FileName("a/target"), "target");

  EXPECT_TRUE(util::StartsWith("abc", "a"));
  EXPECT_TRUE(util::StartsWith("abc", "ab"));
  EXPECT_TRUE(util::StartsWith("abc", "abc"));
  EXPECT_TRUE(util::EndsWith("abc", "c"));
  EXPECT_TRUE(util::EndsWith("abc", "bc"));
  EXPECT_TRUE(util::EndsWith("abc", "abc"));
  EXPECT_TRUE(util::EndsWith("abc", "c"));

  EXPECT_EQ(util::FileDir("abc.png"), "");
  EXPECT_EQ(util::FileDir("/abc.png"), "");
  EXPECT_EQ(util::FileDir("a/abc.png"), "a");

  EXPECT_EQ(util::ReplaceBackslash("a\\abc.png"), "a/abc.png");
  EXPECT_EQ(util::ReplaceBackslash("\\"), "/");
  EXPECT_EQ(util::ReplaceBackslash("a/abc.png"), "a/abc.png");

  glm::vec3 array[] = {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}};
  EXPECT_EQ(util::AsVector(array).size(), 3);
}