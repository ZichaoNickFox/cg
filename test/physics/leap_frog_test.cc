#include "gtest/gtest.h"

#include "physics/leap_frog.h"

namespace {

TEST(physics_leap_frog_test, StoresIntegrationCoefficients) {
  const physics::LeapFrog leap_frog{
      .neg_half = -0.5f,
      .zero = 0.0f,
      .half = 0.5f,
      .one = 1.0f,
  };

  EXPECT_FLOAT_EQ(leap_frog.neg_half, -0.5f);
  EXPECT_FLOAT_EQ(leap_frog.zero, 0.0f);
  EXPECT_FLOAT_EQ(leap_frog.half, 0.5f);
  EXPECT_FLOAT_EQ(leap_frog.one, 1.0f);
}

}  // namespace
