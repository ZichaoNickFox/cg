#include "playground/scene/path_tracing_scene.h"

#include "glm/gtx/string_cast.hpp"
#include <memory>

#include "renderer/color.h"
#include "renderer/math.h"
#include "renderer/meshes/lines_mesh.h"
#include "renderer/scene_common.h"
#include "renderer/shaders.h"
#include "renderer/transform.h"

using namespace renderer;

void PathTracingScene::OnEnter() {
  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{glm::vec3(0.286482, 0.708117, -1.065640)},
                          glm::quat(0.070520, {-0.005535, -0.994436, -0.078057}), {1, 1, 1}});

  // path tracing
  glm::ivec2 viewport_size = io_->screen_size();
  std::vector<glm::vec4> canvas(viewport_size.x * viewport_size.y);
  for (glm::vec4& elem : canvas) {
    elem = kBlack;
  }

  canvas_ = texture_repo_.CreateTexture({viewport_size.x, viewport_size.y, canvas, GL_NEAREST, GL_NEAREST});

  RaytracingDebugCommon::LightPath light_path;

  object_repo_.AddOrReplace(*config_, object_metas_, &mesh_repo_, &material_repo_, &texture_repo_);
  object_repo_.GetPrimitives(mesh_repo_, material_repo_, {Filter::kExcludes, {"sphere"}}, &primitive_repo_);
  bvh_.Build(primitive_repo_, {5, BVH::Partition::kPos, 64});

  glEnable_(GL_DEPTH_TEST);
}

void PathTracingScene::OnUpdate() {
}

void PathTracingScene::OnRender() {
  Rasterization();
  //PathTracing();
}

void PathTracingScene::OnExit() {
}

void PathTracingScene::Rasterization() {
  for (const Object& object : object_repo_.GetObjects({Filter::kExcludes, {"sphere"}})) {
    ColorShader({material_repo_.GetMaterial(object.material_index).diffuse}, *this, object);
  }

/*
  LinesShader({}, *this, LinesMesh(bvh_.GetAABBs()));
*/

  Ray pick_ray = camera_->GetPickRay(io_->GetCursorPosSS());
  RayBVHResult result = RayBVH(pick_ray, bvh_, primitive_repo_);
  if (result.hitted) {
    LinesShader({}, *this, LinesMesh({primitive_repo_.GetTriangle(result.primitive_index)}, kRed));
    LinesShader({}, *this, LinesMesh({result.aabb}, kGreen));
  }
  LinesShader({}, *this, CoordinatorMesh());
}

void PathTracingScene::PathTracing() {
  PathTracingShader::Param param;
  param.screen_size = io_->screen_size();
  param.camera = camera_.get();
  param.frame_num = frame_stat_->frame_num();
  param.output = canvas_;

  PathTracingShader(param, *this);
  RaytracingDebugCommon(canvas_, *this, light_path_ssbo_.GetData<RaytracingDebugCommon::LightPath>());
}
