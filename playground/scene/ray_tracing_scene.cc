#include "playground/scene/ray_tracing_scene.h"

#include "engine/transform.h"
#include "engine/math.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void RayTracingScene::OnEnter(Context* context) {
  for (auto& pair : sphere_map_) {
    const std::string& name = pair.first;
    const engine::Sphere& sphere = pair.second;
    const glm::vec3& translation = sphere.translation;
    const float& r = sphere.radius;
    sphere_object_map_[name].mutable_transform()->SetTranslation(translation);
    sphere_object_map_[name].mutable_transform()->SetScale(glm::vec3(r, r, r));
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

  RaytracingDebugCommon::LightPath light_path;
  light_path_ssbo_.Init(0, light_path);

  automic_counter_.Init(0);
  automic_counter_.Reset(0);

  glEnable_(GL_DEPTH_TEST);
}

void RayTracingScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "PathTracing");
}

void RayTracingScene::OnRender(Context* context) {
  //Resterization(context);
  //RayTracing(context);
  PathTracing(context);
}

void RayTracingScene::OnExit(Context* context) {
}

void RayTracingScene::Resterization(Context* context) {
  for (auto& pair : sphere_map_) {
    const std::string& name = pair.first;
    const engine::Sphere& sphere = pair.second;
    const glm::vec4& color = sphere.color;
    ColorShader({color}, context, &sphere_object_map_[name]);
    sphere_object_map_[name].OnRender(context);
  }

  OnRenderCommon _(context);
}

void RayTracingScene::RayTracing(Context* context) {
  RayTracingShader({context->io().screen_size(), camera_.get(),
                   util::AsValueVector(sphere_map_), canvas_}, context);
  RaytracingDebugCommon(canvas_, context, light_path_ssbo_.GetData<RaytracingDebugCommon::LightPath>());
}

void RayTracingScene::PathTracing(Context* context) {
  PathTracingShader({context->io().screen_size(), camera_.get(),
                    util::AsValueVector(sphere_map_), canvas_}, context);
  RaytracingDebugCommon(canvas_, context, light_path_ssbo_.GetData<RaytracingDebugCommon::LightPath>());
}
