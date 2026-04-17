#include "gtest/gtest.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include "base/util.h"

namespace {

TEST(base_util_test, FileHelpersHandleSeparatorsAndEmptyParts) {
  EXPECT_EQ(util::FileExt("a.txt"), "txt");
  EXPECT_EQ(util::FileExt("a.fs.glsl"), "glsl");
  EXPECT_EQ(util::FileExt("a"), "");

  EXPECT_EQ(util::FileName("a/target.png"), "target");
  EXPECT_EQ(util::FileName("target.png"), "target");
  EXPECT_EQ(util::FileName("target"), "target");

  EXPECT_EQ(util::FileDir("abc.png"), "");
  EXPECT_EQ(util::FileDir("/abc.png"), "");
  EXPECT_EQ(util::FileDir("a/abc.png"), "a");

  EXPECT_EQ(util::ReplaceBackslash("a\\abc.png"), "a/abc.png");
  EXPECT_EQ(util::FileJoin("", "target.png"), "target.png");
  EXPECT_EQ(util::FileJoin("asset", "target.png"), "asset/target.png");
  EXPECT_EQ(util::FileJoin("asset/", "target.png"), "asset/target.png");
  EXPECT_EQ(util::FileJoin("asset\\", "target.png"), "asset/target.png");
  EXPECT_EQ(util::FileJoin("asset", ""), "asset");
}

TEST(base_util_test, StringHelpersTrimInTheExpectedDirection) {
  EXPECT_TRUE(util::StartsWith("shader.glsl", "shader"));
  EXPECT_TRUE(util::EndsWith("shader.glsl", ".glsl"));

  EXPECT_EQ(util::TrimLeft("  shader  "), "shader  ");
  EXPECT_EQ(util::TrimRight("  shader  "), "  shader");
  EXPECT_EQ(util::Trim("  shader  "), "shader");
  EXPECT_EQ(util::Trim("    "), "");
}

TEST(base_util_test, ContainerHelpersHandleNonTrivialTypes) {
  int raw_values[] = {1, 2, 3};
  EXPECT_EQ(util::AsVector(raw_values), std::vector<int>({1, 2, 3}));

  std::vector<int> values = {1, 2, 3, 4, 4, 3, 2, 1};
  util::Remove(3, &values);
  EXPECT_EQ(values, std::vector<int>({1, 2, 4, 4, 2, 1}));

  std::vector<std::string> source = {"zero", "one", "two", "three"};
  std::vector<std::string> destination = {"keep"};
  util::VectorOverride(&destination, 1, source, 1, 4);
  EXPECT_EQ(destination, std::vector<std::string>({"keep", "one", "two", "three"}));
}

TEST(base_util_test, MakeDirAndReadFileToStringWorkOnNestedPaths) {
  const auto unique_suffix =
      std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
  const std::filesystem::path temp_root =
      std::filesystem::temp_directory_path() / ("cg-util-test-" + unique_suffix);
  const std::filesystem::path nested_dir = temp_root / "nested" / "path";
  const std::filesystem::path file_path = nested_dir / "sample.txt";

  util::MakeDir(nested_dir.string());
  ASSERT_TRUE(std::filesystem::exists(nested_dir));

  {
    std::ofstream output(file_path);
    output << "shader-loader";
  }

  std::string content;
  util::ReadFileToString(file_path.string(), &content);
  EXPECT_EQ(content, "shader-loader");

  std::filesystem::remove_all(temp_root);
}

TEST(base_util_test, FloatEqUsesMachineEpsilon) {
  EXPECT_TRUE(util::FloatEq(1.0f, 1.0f));
  EXPECT_TRUE(util::FloatEq(1.0f + std::numeric_limits<float>::epsilon() / 2.0f, 1.0f));
  EXPECT_FALSE(util::FloatEq(1.001f, 1.0f));
}

}  // namespace
