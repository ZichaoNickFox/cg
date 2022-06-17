#include "playground/scene/path_tracing_scene.h"

#include "glm/gtx/string_cast.hpp"

#include "engine/transform.h"
#include "engine/math.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void PathTracingScene::OnEnter(Context* context) {
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
  light_path_ssbo_.Init(0, sizeof(light_path), &light_path);

  std::vector<engine::BVH::Primitive> primitives;
  for (auto& p : conell_box_) {
    p.second.object.Init(context, p.first, p.first);
    std::vector<engine::AABB> model_aabbs = p.second.object.GetAABBs(context);
    for (const engine::AABB& model_aabb : model_aabbs) {
      primitives.push_back({model_aabb, p.second.primitive_index});
    }
  }
  bvh_.Build(&primitives, {3, 12});

  //std::vector<engine::AABB> aabbs = sphere_.GetAABBs(context);
  //std::vector<engine::BVH::Primitive> primitives(aabbs.size());
  //const int kSpherePrimitiveIndex = 0;
  //for (int i = 0; i < primitives.size(); ++i) {
  //  primitives[i] = {aabbs[i], kSpherePrimitiveIndex};
  //}
  //bvh_.Build(&primitives, {5, 64});

  glEnable_(GL_DEPTH_TEST);
}

void PathTracingScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "PathTracing");
}

void PathTracingScene::OnRender(Context* context) {
  Rasterization(context);
  //PathTracing(context);
}

void PathTracingScene::OnExit(Context* context) {
}

void PathTracingScene::Rasterization(Context* context) {
  //ColorShader({glm::vec4(1, 1, 1, 1)}, context, &sphere_);
  //sphere_.OnRender(context);

  for (auto& p : conell_box_) {
    ColorShader _({p.second.color}, context, &p.second.object);
    p.second.object.OnRender(context);
  }

  std::vector<engine::AABB> aabbs = bvh_.GetAABBs();
  LinesObject lines_object;
  LinesObject::Mesh lines_mesh(aabbs, glm::vec4(0, 0, 1, 1));
  lines_object.SetMesh(lines_mesh);
  LinesShader lines_shader({}, context, &lines_object);
  lines_object.OnRender(context);
}

void PathTracingScene::PathTracing(Context* context) {
  PathTracingShader::Param param;
  param.screen_size = context->io().screen_size();
  param.camera = camera_.get();
  param.frame_num = context->frame_stat().frame_num();
  param.output = canvas_;

  PathTracingShader(param, context);
  RaytracingDebugCommon(canvas_, context, light_path_ssbo_.GetData<RaytracingDebugCommon::LightPath>());
}
