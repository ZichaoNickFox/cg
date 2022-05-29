#include "playground/scene/path_tracing_scene.h"

#include "engine/transform.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void PathTracingScene::OnEnter(Context* context) {
  light_.SetTransform({{0, 5.4, 1}, glm::quat(), {3, 3, 3}});
  ground_.SetTransform({{0, -100.5, 1}, glm::quat(), {100, 100, 100}});
  ceiling_.SetTransform({{0, 102.5, 1}, glm::quat(), {100, 100, 100}});
  back_wall_.SetTransform({{0, 1, -101}, glm::quat(), {100, 100, 100}});
  left_wall_.SetTransform({{-101.5, 0, 1}, glm::quat(), {100, 100, 100}});
  right_wall_.SetTransform({{101.5, 0, 1}, glm::quat(), {100, 100, 100}});

  diffuse_ball_.SetTransform({{0, -0.2, 1.5}, glm::quat(), {0.3, 0.3, 0.3}});
  metal_ball_.SetTransform({{0.8, 0.2, 1}, glm::quat(), {0.7, 0.7, 0.7}});
  glass_ball_.SetTransform({{-0.7, 0, 0.5}, glm::quat(), {0.5, 0.5, 0.5}});
  metal_ball2_.SetTransform({{-0.6, -0.3, 2}, glm::quat(), {0.2, 0.2, 0.2}});

  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{0, 1, 5}, glm::quat(1, -0.02, 0, 0), {1, 1, 1}});
  context->SetCamera(camera_.get());

  glEnable_(GL_DEPTH_TEST);
}

void PathTracingScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "PathTracing");
}

void PathTracingScene::OnRender(Context* context) {
  ColorShader({glm::vec4(10, 10, 10, 10)}, context, &light_);
  light_.OnRender(context);
  ColorShader({glm::vec4(0.8, 0.8, 0.8, 1.0)}, context, &ground_);
  ground_.OnRender(context);
  ColorShader({glm::vec4(0.8, 0.8, 0.8, 1.0)}, context, &ceiling_);
  ceiling_.OnRender(context);
  ColorShader({glm::vec4(0.8, 0.8, 0.8, 1.0)}, context, &back_wall_);
  back_wall_.OnRender(context);
  ColorShader({glm::vec4(0.0, 0.6, 0.0, 1.0)}, context, &left_wall_);
  left_wall_.OnRender(context);
  ColorShader({glm::vec4(0.6, 0.0, 0.0, 1.0)}, context, &right_wall_);
  right_wall_.OnRender(context);

  ColorShader({glm::vec4(0.8, 0.3, 0.3, 1.0)}, context, &diffuse_ball_);
  diffuse_ball_.OnRender(context);
  ColorShader({glm::vec4(0.6, 0.8, 0.8, 1.0)}, context, &metal_ball_);
  metal_ball_.OnRender(context);
  ColorShader({glm::vec4(1.0, 1.0, 1.0, 1.0)}, context, &glass_ball_);
  glass_ball_.OnRender(context);
  ColorShader({glm::vec4(0.8, 0.6, 0.2, 1.0)}, context, &metal_ball2_);
  metal_ball2_.OnRender(context);
  OnRenderCommon _(context);
}

void PathTracingScene::OnExit(Context* context) {
  ground_.OnDestory(context);
}

