#include "gtest/gtest.h"

#include <vector>

#include "base/geometry.h"

namespace {

void ExpectVec2Near(const glm::vec2& actual, const glm::vec2& expected, float epsilon = 1e-5f) {
  EXPECT_NEAR(actual.x, expected.x, epsilon);
  EXPECT_NEAR(actual.y, expected.y, epsilon);
}

void ExpectVec3Near(const glm::vec3& actual, const glm::vec3& expected, float epsilon = 1e-5f) {
  EXPECT_NEAR(actual.x, expected.x, epsilon);
  EXPECT_NEAR(actual.y, expected.y, epsilon);
  EXPECT_NEAR(actual.z, expected.z, epsilon);
}

TEST(base_geometry_test, FootOfPerpendicularProjectsOntoTheSegmentDirection) {
  ExpectVec3Near(
      cg::GetFootOfPerpendicular(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(2.0f, 0.0f, 0.0f)),
      glm::vec3(1.0f, 0.0f, 0.0f));
  ExpectVec3Near(
      cg::GetFootOfPerpendicular(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(2.0f), glm::vec3(2.0f)),
      glm::vec3(2.0f));
}

TEST(base_geometry_test, AabbCalculationsUseAllThreeAxes) {
  cg::AABB aabb{
      .maximum = glm::vec3(5.0f, 7.0f, 11.0f),
      .minimum = glm::vec3(1.0f, 2.0f, 3.0f),
  };

  EXPECT_FLOAT_EQ(aabb.SurfaceArea(), 184.0f);
  ExpectVec3Near(aabb.Center(), glm::vec3(3.0f, 4.5f, 7.0f));
  EXPECT_EQ(aabb.GetMaxLengthAxis(), cg::AABB::kZ);
  EXPECT_FLOAT_EQ(aabb.GetLengthByAxis(cg::AABB::kY), 5.0f);
}

TEST(base_geometry_test, RayAabbHandlesParallelRaysCorrectly) {
  const cg::AABB aabb{
      .maximum = glm::vec3(1.0f),
      .minimum = glm::vec3(0.0f),
  };

  EXPECT_FALSE(cg::RayAABB(cg::Ray{.position = glm::vec3(2.0f, 0.5f, 0.5f), .direction = glm::vec3(0.0f, 1.0f, 0.0f)},
                           aabb)
                   .hitted);
  EXPECT_TRUE(cg::RayAABB(cg::Ray{.position = glm::vec3(0.5f, -1.0f, 0.5f), .direction = glm::vec3(0.0f, 1.0f, 0.0f)},
                          aabb)
                  .hitted);
}

TEST(base_geometry_test, RayTriangleIntersectionReturnsDistancePositionAndNormal) {
  const cg::Triangle triangle{
      .a = glm::vec3(-1.0f, -1.0f, -5.0f),
      .b = glm::vec3(1.0f, -1.0f, -5.0f),
      .c = glm::vec3(0.0f, 1.0f, -5.0f),
  };

  const cg::RayTriangleResult result =
      cg::RayTriangle(cg::Ray{.position = glm::vec3(0.0f), .direction = glm::vec3(0.0f, 0.0f, -1.0f)}, triangle);
  ASSERT_TRUE(result.hitted);
  EXPECT_FLOAT_EQ(result.distance, 5.0f);
  ExpectVec3Near(result.position, glm::vec3(0.0f, 0.0f, -5.0f));
  ExpectVec3Near(result.normal, glm::vec3(0.0f, 0.0f, 1.0f));
  EXPECT_FLOAT_EQ(triangle.GetArea(), 2.0f);
}

TEST(base_geometry_test, TriangleAabbAddsThicknessForFlatTriangles) {
  const cg::Triangle triangle{
      .a = glm::vec3(0.0f, 0.0f, 0.0f),
      .b = glm::vec3(1.0f, 0.0f, 0.0f),
      .c = glm::vec3(0.0f, 1.0f, 0.0f),
  };

  const cg::AABB aabb = triangle.AsAABB();
  EXPECT_GT(aabb.maximum.z, aabb.minimum.z);
}

TEST(base_geometry_test, BresenhamVisitsBothEndpoints) {
  const cg::LineSegment line(glm::vec2(0.0f, 0.0f), glm::vec2(3.0f, 0.0f));
  std::vector<glm::vec2> visited;
  line.Bresenham(10, 10, [&visited](const glm::vec2& point) { visited.push_back(point); });

  ASSERT_EQ(visited.size(), 4);
  ExpectVec2Near(visited.front(), glm::vec2(0.0f, 0.0f));
  ExpectVec2Near(visited[1], glm::vec2(1.0f, 0.0f));
  ExpectVec2Near(visited[2], glm::vec2(2.0f, 0.0f));
  ExpectVec2Near(visited.back(), glm::vec2(3.0f, 0.0f));
}

}  // namespace
