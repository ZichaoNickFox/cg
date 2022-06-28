#include "playground/scene/ray_tracing_scene.h"

#include "renderer/filter.h"
#include "renderer/math.h"
#include "renderer/object.h"
#include "renderer/scene_common.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

using namespace renderer;

void RayTracingScene::OnEnter() {
  for (auto& pair : sphere_map_) {
    const std::string& name = pair.first;
    const Sphere& sphere = pair.second;
    const glm::vec3& translation = sphere.translation;
    const float& r = sphere.radius;

    ObjectMeta object_meta{name, {glm::vec3(), glm::quat(), glm::vec3(r, r, r)}, name, name};
    object_repo_.AddOrReplace(*config_, object_meta, &mesh_repo_, &material_repo_, &texture_repo_);
  }

  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{0, 1, 5}, glm::quat(1, -0.02, 0, 0), {1, 1, 1}});

  // path tracing
  glm::ivec2 viewport_size = io_->screen_size();
  std::vector<glm::vec4> canvas(viewport_size.x * viewport_size.y, kBlack);
  canvas_ = CreateTexture2D(viewport_size.x, viewport_size.y, canvas, GL_NEAREST, GL_NEAREST);

  RaytracingDebugCommon::LightPath light_path;

  glEnable_(GL_DEPTH_TEST);
}

void RayTracingScene::OnUpdate() {
}

void RayTracingScene::OnRender() {
  //Resterization();
  //RayTracing();
  PathTracing();
}

void RayTracingScene::OnExit() {
}

void RayTracingScene::Resterization() {
  for (auto& pair : sphere_map_) {
    const std::string& name = pair.first;
    const glm::vec4& color = pair.second.color;
    ColorShader({color}, *this, object_repo_.GetObject(name));
  }
}

void RayTracingScene::RayTracing() {
  RayTracingShader({io_->screen_size(), camera_.get(),
                   util::AsValueVector(sphere_map_), canvas_}, *this);
  RaytracingDebugCommon(canvas_, *this, light_path_ssbo_.GetData<RaytracingDebugCommon::LightPath>());
}

void RayTracingScene::PathTracing() {
  PathTracingDemoShader({io_->screen_size(), camera_.get(), util::AsValueVector(sphere_map_),
                         frame_stat_->frame_num(), canvas_}, *this);
  RaytracingDebugCommon(canvas_, *this, light_path_ssbo_.GetData<RaytracingDebugCommon::LightPath>());
}
