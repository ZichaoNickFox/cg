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
    renderer::Texture texture_in_out;
    bool dirty = false;
  };
  PathTracingShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "path_tracing_scene") {
    SetCamera(scene.camera());
    SetTextureBinding({param.texture_in_out, "texture_in_out", GL_READ_WRITE});
    SetFrameNum(scene);
    SetWorkGroupNum({scene.io().screen_size().x / 32, scene.io().screen_size().y / 32, 1});
    SetResolution({scene.io().screen_size().x, scene.io().screen_size().y});
    SetDirty(param.dirty);
    Run();
  }
};

void PathTracingScene::OnEnter() {
  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{glm::vec3(0.286482, 0.708117, -1.065640)},
                          glm::quat(0.070520, {-0.005535, -0.994436, -0.078057}), {1, 1, 1}});

  // path tracing
  glm::vec3 canvas_size(3240, 2160, 0);
  std::vector<glm::vec4> canvas_data(canvas_size.x * canvas_size.y, kBlack);
  texture_in_out_ = CreateTexture2D(canvas_size.x, canvas_size.y, canvas_data);

  object_repo_.AddOrReplace(object_metas_);
  object_repo_.BreakIntoPrimitives(mesh_repo_, material_repo_, {}, &primitive_repo_);
  bvh_.Build(primitive_repo_, {100, BVH::Partition::kPos, 64});

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
  bool dirty = !(dirty_camera_transform_ == camera_->transform());
  dirty_camera_transform_ = camera_->transform();
  PathTracingShader({texture_in_out_, dirty}, *this);
  FullscreenQuadShader({texture_in_out_}, *this);
}
