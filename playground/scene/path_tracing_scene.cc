#include "playground/scene/path_tracing_scene.h"

#include "engine/transform.h"
#include "engine/math.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void PathTracingScene::OnEnter(Context* context) {
  for (auto& pair : sphere_geometry_map_) {
    const std::string& name = pair.first;
    const engine::SphereGeometry& sphere_data = pair.second;
    const glm::vec3& translation = sphere_data.translation;
    const float& r = sphere_data.radius;
    sphere_map_[name].mutable_transform()->SetTranslation(translation);
    sphere_map_[name].mutable_transform()->SetScale(glm::vec3(r, r, r));
  }

  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{0, 1, 5}, glm::quat(1, -0.02, 0, 0), {1, 1, 1}});
  context->SetCamera(camera_.get());

  // path tracing
  glm::ivec2 viewport_size = context->io().screen_size();
  std::vector<glm::vec4> canvas(viewport_size.x * viewport_size.y);
  for (glm::vec4& elem : canvas) {
    elem = glm::vec4(0, 0, 0, 1);
  }
  canvas_ = context->CreateTexture({viewport_size.x, viewport_size.y, canvas, GL_NEAREST, GL_NEAREST});

  glEnable_(GL_DEPTH_TEST);
}

void PathTracingScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "PathTracing");
}

void PathTracingScene::OnRender(Context* context) {
  //Resterization(context);
  //RayTracing(context);
  PathTracing(context);
}

void PathTracingScene::OnExit(Context* context) {
}

void PathTracingScene::Resterization(Context* context) {
  for (auto& pair : sphere_geometry_map_) {
    const std::string& name = pair.first;
    const engine::SphereGeometry& sphere_data = pair.second;
    const glm::vec4& color = sphere_data.color;
    ColorShader({color}, context, &sphere_map_[name]);
    sphere_map_[name].OnRender(context);
  }

  OnRenderCommon _(context);
}

void PathTracingScene::RayTracing(Context* context) {
  RayTracingShader({context->io().screen_size(), camera_.get(),
                   util::AsValueVector(sphere_geometry_map_), canvas_}, context);
  TextureDebugFullScreen(canvas_, context);
}

void PathTracingScene::PathTracing(Context* context) {
  PathTracingShader({context->io().screen_size(), camera_.get(),
                   util::AsValueVector(sphere_geometry_map_), canvas_}, context);
  TextureDebugFullScreen(canvas_, context);
}
