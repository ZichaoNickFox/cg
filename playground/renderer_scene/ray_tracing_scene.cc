#include "playground/renderer_scene/ray_tracing_scene.h"

#include "base/math.h"
#include "renderer/filter.h"
#include "renderer/inspector.h"
#include "renderer/object.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

using namespace renderer;

class RayTracingShader : public renderer::ComputeShader {
 public:
  struct Param {
    std::vector<renderer::Sphere> spheres;
    renderer::Texture canvas;
  };
  RayTracingShader(const Param& param, const Scene& scene)
      : ComputeShader(scene, "ray_tracing") {
    SetCamera(scene.camera());
    for (int i = 0; i < param.spheres.size(); ++i) {
      const Sphere& sphere = param.spheres[i];
      program_.SetInt(std::format("spheres[{}].id", i), sphere.id);
      program_.SetVec3(std::format("spheres[{}].center_pos", i), sphere.translation);
      program_.SetVec4(std::format("spheres[{}].color", i), sphere.color);
      program_.SetFloat(std::format("spheres[{}].radius", i), sphere.radius);
    }
    SetTextureBinding({param.canvas, "canvas", GL_READ_ONLY});
    Run();
  }
};

void RayTracingScene::OnEnter() {
  for (auto& pair : sphere_map_) {
    const std::string& name = pair.first;
    const Sphere& sphere = pair.second;
    const glm::vec3& translation = sphere.translation;
    const float& r = sphere.radius;

    ObjectMeta object_meta{name, {glm::vec3(), glm::quat(), glm::vec3(r, r, r)}, name, name};
    object_repo_.AddOrReplace(object_meta);
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
  RayTracing();
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
  RayTracingShader({util::AsValueVector(sphere_map_), canvas_}, *this);
  RaytracingDebugCommon(canvas_, *this, light_path_ssbo_.GetData<RaytracingDebugCommon::LightPath>());
}