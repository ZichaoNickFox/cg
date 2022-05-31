#include "playground/scene/path_tracing_scene.h"

#include "engine/transform.h"
#include "engine/math.h"
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

  // path tracing
  glm::vec2 viewport_size = context->io().screen_size();
  canvas_.resize(viewport_size.x * viewport_size.y);
  for (glm::vec3& elem : canvas_) {
    elem = glm::vec3(0,0,0);
  }
  engine::CreateTexture2DParam param{1, int(viewport_size.x), int(viewport_size.y),
                                     std::vector<void*>{static_cast<void*>(canvas_.data())},
                                     GL_RGB32F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST};
  texture_canvas_ = context->mutable_texture_repo()->CreateTempTexture2D(param);

  glEnable_(GL_DEPTH_TEST);
}

void PathTracingScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "PathTracing");
}

void PathTracingScene::OnRender(Context* context) {
  //Resterization(context);
  RayTracing(context);
}

void PathTracingScene::OnExit(Context* context) {
  ground_.OnDestory(context);
}

void PathTracingScene::Resterization(Context* context) {
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

void PathTracingScene::RayTracing(Context* context) {
/*
  engine::PathTracing path_tracing({});
  glm::vec2 viewport_size = context->io().screen_size();
  int ray_num = viewport_size.x  * viewport_size.y;
  for (int i = 0; i < ray_num; ++i) {
    float u = engine::RandFromTo(0, viewport_size.x - 1) / viewport_size.x;
    float v = engine::RandFromTo(0, viewport_size.y - 1) / viewport_size.y;
    glm::vec3 near, far;
    engine::Ray ray(near, glm::normalize(far - near));
    glm::vec3 color = path_tracing.GetRayColor(ray);
    canvas_[u * viewport_size.x][v * viewport_size.y] = color;
  }
  */

  glm::vec2 viewport_size = context->io().screen_size();
  for (int i = 0; i < viewport_size.x * viewport_size.y; ++i) {
    //canvas_[i] = glm::vec3(engine::RandFromTo(0,1), engine::RandFromTo(0,1), engine::RandFromTo(0,1));
  }
  glBindTexture_(GL_TEXTURE_2D, texture_canvas_.id());
  glTexSubImage2D_(GL_TEXTURE_2D, 0, 0, 0, viewport_size.x, viewport_size.y,
                   GL_RGB, GL_FLOAT, canvas_.data());
  
  EmptyObject object;
  FullscreenQuadShader({texture_canvas_}, context, &object);
  object.OnRender(context);
}
