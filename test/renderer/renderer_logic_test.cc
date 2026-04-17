#include "gtest/gtest.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

#include "renderer/camera.h"
#include "renderer/filter.h"
#include "renderer/frame_stat.h"
#include "renderer/transform.h"

namespace {

void ExpectVec3Near(const glm::vec3& actual, const glm::vec3& expected, float epsilon = 1e-5f) {
  EXPECT_NEAR(actual.x, expected.x, epsilon);
  EXPECT_NEAR(actual.y, expected.y, epsilon);
  EXPECT_NEAR(actual.z, expected.z, epsilon);
}

void ExpectMat4Near(const glm::mat4& actual, const glm::mat4& expected, float epsilon = 1e-5f) {
  for (int column = 0; column < 4; ++column) {
    for (int row = 0; row < 4; ++row) {
      EXPECT_NEAR(actual[column][row], expected[column][row], epsilon);
    }
  }
}

TEST(renderer_logic_test, FilterPassesAccordingToMode) {
  EXPECT_TRUE(cg::Filter().Pass("anything"));

  const cg::Filter includes(cg::Filter::kIncludes, {"mesh", "light"});
  EXPECT_TRUE(includes.Pass("mesh"));
  EXPECT_FALSE(includes.Pass("camera"));

  const cg::Filter excludes(cg::Filter::kExcludes, {"debug"});
  EXPECT_TRUE(excludes.Pass("mesh"));
  EXPECT_FALSE(excludes.Pass("debug"));
}

TEST(renderer_logic_test, FrameStatMaintainsRollingAverage) {
  cg::FrameStat frame_stat;
  frame_stat.OnFrame(10);
  frame_stat.OnFrame(20);
  frame_stat.OnFrame(30);

  EXPECT_EQ(frame_stat.frame_num(), 3);
  EXPECT_EQ(frame_stat.last_frame_interval(), 30);
  EXPECT_EQ(frame_stat.last_fps(), 33);
  EXPECT_EQ(frame_stat.avg_frame_interval(), 20);
  EXPECT_EQ(frame_stat.avg_fps(), 50);

  for (int i = 0; i < 60; ++i) {
    frame_stat.OnFrame(10);
  }
  EXPECT_EQ(frame_stat.frame_num(), 63);
  EXPECT_EQ(frame_stat.avg_frame_interval(), 10);
}

TEST(renderer_logic_test, TransformProducesTranslateRotateScaleMatrix) {
  cg::Transform transform(glm::vec3(1.0f, 2.0f, 3.0f),
                          glm::angleAxis(glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f)),
                          glm::vec3(2.0f, 3.0f, 4.0f));

  const glm::vec4 transformed_point = transform.GetModelMatrix() * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  EXPECT_NEAR(transformed_point.x, 1.0f, 1e-5f);
  EXPECT_NEAR(transformed_point.y, 4.0f, 1e-5f);
  EXPECT_NEAR(transformed_point.z, 3.0f, 1e-5f);
  EXPECT_NEAR(transformed_point.w, 1.0f, 1e-5f);
}

TEST(renderer_logic_test, CameraProjectViewAndPickRayMatchGlm) {
  cg::Camera camera;
  camera.SetAspect(1.0f);
  camera.SetPerspectiveFov(90.0f);
  camera.SetNearClip(0.1f);
  camera.SetFarClip(100.0f);

  ExpectMat4Near(camera.GetProjectMatrix(), glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f));
  ExpectMat4Near(camera.GetViewMatrix(), glm::mat4(1.0f));

  const cg::Ray pick_ray = camera.GetPickRay(glm::vec2(0.5f, 0.5f));
  ExpectVec3Near(pick_ray.direction, glm::vec3(0.0f, 0.0f, -1.0f), 1e-4f);
  EXPECT_LT(pick_ray.position.z, 0.0f);

  camera.SetType(cg::Camera::Orthographic);
  camera.SetAspect(2.0f);
  camera.SetOrthographicWidth(8.0f);
  ExpectMat4Near(camera.GetProjectMatrix(), glm::ortho(-4.0f, 4.0f, -2.0f, 2.0f, 0.1f, 100.0f));
}

}  // namespace
