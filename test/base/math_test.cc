#include "gtest/gtest.h"

#include <algorithm>
#include <vector>

#include "base/math.h"

namespace {

TEST(base_math_test, LerpClampAndLinspaceHandleInclusiveEndpoints) {
  EXPECT_FLOAT_EQ(math::Lerp(2.0f, 10.0f, 0.25f), 4.0f);
  EXPECT_FLOAT_EQ(math::Clamp(-3.0f, 0.0f, 1.0f), 0.0f);
  EXPECT_FLOAT_EQ(math::Clamp(0.5f, 0.0f, 1.0f), 0.5f);
  EXPECT_FLOAT_EQ(math::Clamp(3.0f, 0.0f, 1.0f), 1.0f);

  EXPECT_EQ(math::Linspace(-2.0f, 2.0f, 1), std::vector<float>({-2.0f}));
  EXPECT_EQ(math::Linspace(0.0f, 1.0f, 3), std::vector<float>({0.0f, 0.5f, 1.0f}));
  EXPECT_EQ(math::Linspace(2.0f, 8.0f, 4), std::vector<float>({2.0f, 4.0f, 6.0f, 8.0f}));
}

TEST(base_math_test, TransformMinMaxAndStd2EigenPreserveValues) {
  std::vector<float> values = {1.0f, -2.0f, 3.0f};
  EXPECT_EQ(math::Transform(values, [](float value) { return value * value; }),
            std::vector<float>({1.0f, 4.0f, 9.0f}));
  EXPECT_FLOAT_EQ(math::Min(values), -2.0f);
  EXPECT_FLOAT_EQ(math::Max(values), 3.0f);

  Eigen::VectorXf eigen_values = math::Std2Eigen(values);
  ASSERT_EQ(eigen_values.size(), 3);
  EXPECT_FLOAT_EQ(eigen_values(0), 1.0f);
  EXPECT_FLOAT_EQ(eigen_values(1), -2.0f);
  EXPECT_FLOAT_EQ(eigen_values(2), 3.0f);
}

TEST(base_math_test, QuickSelectPlacesNthElementWithoutFullySorting) {
  std::vector<int> values = {4, 8, 2, 3, 5, 7, 9, 6, 1};
  const int index = 4;
  math::QuickSelect(&values, 0, values.size(), index, [](int left, int right) { return left <= right; });

  EXPECT_EQ(values[index], 5);
  for (int i = 0; i < index; ++i) {
    EXPECT_LE(values[i], values[index]);
  }
  for (int i = index + 1; i < values.size(); ++i) {
    EXPECT_GE(values[i], values[index]);
  }
}

TEST(base_math_test, QuickSelectHandlesDuplicateValues) {
  std::vector<int> values = {9, 1, 5, 5, 5, 2, 7};
  const int index = 3;
  math::QuickSelect(&values, 0, values.size(), index, [](int left, int right) { return left <= right; });

  EXPECT_EQ(values[index], 5);
  for (int i = 0; i < index; ++i) {
    EXPECT_LE(values[i], values[index]);
  }
  for (int i = index + 1; i < values.size(); ++i) {
    EXPECT_GE(values[i], values[index]);
  }
}

}  // namespace
