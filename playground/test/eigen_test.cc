#include "gtest/gtest.h"

#include "base/debug.h"
#include <eigen/core>
#include <eigen/LU>

using namespace std;
using namespace Eigen;

TEST(eigen_test, tutorial) {
  ASSERT_EQ(EIGEN_MAJOR_VERSION, 4);
  ASSERT_EQ(EIGEN_WORLD_VERSION, 3);

  Matrix<short, 5, 5> m1;
  Matrix<float, 20, 75> m2;
  Matrix4f m3 = Matrix4f::Random();
  Matrix4f m4 = Matrix4f::Ones();
  Matrix4f m5 = Matrix4f::Zero();
  Matrix4f m6 = m3.transpose();
  Matrix4f m7 = m3.inverse();
  ASSERT_NE(m3, m6);
  ASSERT_NE(m3, m7);

  Matrix3f a;
  a << 1.0f, 0.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 0.0f, 1.0f;
  ASSERT_EQ(a, Matrix3f::Identity());

  ASSERT_EQ(a * 5 * Vector3f(1, 2, 3), Vector3f(5, 10, 15));
}