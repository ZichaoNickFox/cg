#include "playground/scene/path_tracing_scene.h"

#include "glm/gtx/string_cast.hpp"
#include <memory>

#include "renderer/color.h"
#include "renderer/inspector.h"
#include "renderer/math.h"
#include "renderer/mesh/lines_mesh.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

using namespace renderer;

class PathTracingShader : public renderer::ComputeShader {
 public:
  struct Param {
    std::vector<renderer::Sphere> spheres;
    renderer::Texture canvas;
  };
  PathTracingShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "path_tracing_scene") {
    SetCamera(scene.camera());
    SetScreenSize(scene.io().screen_size());
    SetTextureBinding({param.canvas, "canvas", GL_READ_WRITE, GL_RGBA32F});
    SetTimeSeed(scene.frame_stat().frame_num());
    for (int i = 0; i < param.spheres.size(); ++i) {
      const Sphere& sphere = param.spheres[i];
      program_.SetInt(fmt::format("spheres[{}].id", i), sphere.id);
      program_.SetVec3(fmt::format("spheres[{}].center_pos", i), sphere.translation);
      program_.SetVec4(fmt::format("spheres[{}].color", i), sphere.color);
      program_.SetFloat(fmt::format("spheres[{}].radius", i), sphere.radius);
    }
    Run();
  }
};

void PathTracingScene::OnEnter() {
  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{glm::vec3(0.286482, 0.708117, -1.065640)},
                          glm::quat(0.070520, {-0.005535, -0.994436, -0.078057}), {1, 1, 1}});

  // path tracing
  glm::ivec2 framebuffer_size = io_->framebuffer_size();
  std::vector<glm::vec4> canvas(framebuffer_size.x * framebuffer_size.y, kBlack);
  canvas_ = CreateTexture2D(framebuffer_size.x, framebuffer_size.y, canvas);

  object_repo_.AddOrReplace(object_metas_);
  object_repo_.GetPrimitives(mesh_repo_, material_repo_, {}, &primitive_repo_);
  bvh_.Build(primitive_repo_, {5, BVH::Partition::kPos, 64});

  glEnable_(GL_DEPTH_TEST);
}

void PathTracingScene::OnUpdate() {
}

void PathTracingScene::OnRender() {
  // Rasterization();
  PathTracing();
}

void PathTracingScene::OnExit() {
}

void PathTracingScene::Rasterization() {
  for (const Object& object : object_repo_.GetObjects({Filter::kExcludes, {"sphere"}})) {
    ColorShader({material_repo_.GetMaterial(object.material_index).diffuse}, *this, object);
  }

  Ray pick_ray = camera_->GetPickRay(io_->GetCursorPosSS());
  RayBVHResult result = RayBVH(pick_ray, bvh_, primitive_repo_);
  if (result.hitted) {
    LinesShader({}, *this, LinesMesh({primitive_repo_.GetTriangle(result.primitive_index)}, kRed));
    LinesShader({}, *this, LinesMesh({result.aabb}, kGreen));
  }
}

void PathTracingScene::PathTracing() {
  PathTracingShader::Param param;
  param.canvas = canvas_;

  PathTracingShader(param, *this);
  FullscreenQuadShader({canvas_}, *this);
}
