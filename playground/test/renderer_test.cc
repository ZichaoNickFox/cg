#include "gtest/gtest.h"

#include <glm/glm.hpp>
#include <map>
#include <math.h>

#include "renderer/math.h"
#include "renderer/util.h"

TEST(renderer_test, util)
{
  EXPECT_EQ(util::FileExt("a.txt"), "txt");
  EXPECT_EQ(util::FileExt("a.fs.glsl"), "glsl");
  EXPECT_EQ(util::FileExt("a"), "");

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

  std::vector<int> vs = {1, 2, 3, 4, 4, 3, 2, 1, 2, 3, 4};
  util::Remove(3, &vs);
  std::vector<int> vs2 = {1, 2, 4, 4, 2, 1, 2, 4};
  EXPECT_EQ(vs, vs2);
}

struct S {int a; float b;};
bool operator == (const S& left, const S& right) {
  return left.a == right.a && left.b == right.b;
}

TEST(renderer_test, VectorOverride) {
  std::vector<S> copy_from = {{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4}};
  std::vector<S> copy_to = {{5, 5.5}, {6, 6.6}, {7, 7.7}};
  util::VectorOverride(&copy_to, 1, copy_from, 1, 3);
  std::vector<S> copy_result = {{5, 5.5}, {2, 2.2}, {3, 3.3}};
  EXPECT_EQ(copy_to, copy_result);
}

TEST(renderer_test, math) {
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    util::QuickSelect(&vs, 0, vs.size(), 3, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    util::QuickSelect(&vs, 0, vs.size(), 0, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    util::QuickSelect(&vs, 0, vs.size(), 1, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 2, 3, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    util::QuickSelect(&vs, 0, vs.size(), 2, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 2, 3, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    util::QuickSelect(&vs, 0, vs.size(), 4, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    util::QuickSelect(&vs, 0, vs.size(), 5, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 6, 7, 9, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    util::QuickSelect(&vs, 0, vs.size(), 8, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    util::QuickSelect(&vs, 0, vs.size(), 7, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    util::QuickSelect(&vs, 0, vs.size(), 6, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 6, 7, 9, 8};
    EXPECT_EQ(vs, target);
  }
}