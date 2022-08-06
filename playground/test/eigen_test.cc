#include "gtest/gtest.h"

#include "base/debug.h"
#include "base/eigen_util.h"
#include <eigen/core>
#include <eigen/Dense>
#include <eigen/LU>
#include <vector>

using namespace std;
using namespace Eigen;

// https://eigen.tuxfamily.org/dox/group__TutorialSlicingIndexing.html
TEST(eigen_test, indexing) {
  Matrix3f A;
  Vector3f b;
  A << 1.0f, 2.0f, 3.0f,
       4.0f, 5.0f, 6.0f,
       7.0f, 8.0f, 10.0f;
  b << 3.0f, 3.0f, 4.0f;
  ASSERT_EQ(A(0, 1), 2.0f);
  ASSERT_EQ(Vector3f(A(0, all)), Vector3f(1.0f, 2.0f, 3.0f));
  ASSERT_EQ(Vector3f(A(all, 0)), Vector3f(1.0f, 4.0f, 7.0f));
}

// https://dritchie.github.io/csci2240/assignments/eigen_tutorial.pdf
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

// https://eigen.tuxfamily.org/dox/group__TutorialLinearAlgebra.html
TEST(eigen_test, linear_solver) {
  Matrix3f A;
  Vector3f b;
  A << 1.0f, 2.0f, 3.0f,
       4.0f, 5.0f, 6.0f,
       7.0f, 8.0f, 10.0f;
  b << 3.0f, 3.0f, 4.0f;
  Vector3f x = A.colPivHouseholderQr().solve(b);
  ASSERT_NEAR(x[0], -2, 0.0001f);
  ASSERT_NEAR(x[1], 1, 0.0001f);
  ASSERT_NEAR(x[2], 1, 0.0001f);
}

TEST(eigen_test, std) {
  std::vector<float> v{1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
  Eigen::VectorXf res = Vector2Eigen(v);
  ASSERT_EQ(res(2), 3.0f);
}

TEST(eigen_test, newton) {
  float a = 2;
  auto f = [&a] (float x) { return x * x - a; };
  auto fd = [] (float x) { return 2 * x; };
  float x1 = 0;
  float x2 = 1;
  while (true) {
    x2 = x1 - f(x1) / fd(x1);
    if (abs(x2 - x1) < 0.001) {
      break;
    }
    x1 = x2;
  }
  ASSERT_NEAR(x2, 1.414, 0.001);
}