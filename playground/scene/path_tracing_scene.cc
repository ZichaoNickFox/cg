#include "playground/scene/path_tracing_scene.h"

#include "glm/gtx/string_cast.hpp"

#include "engine/color.h"
#include "engine/math.h"
#include "engine/transform.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void PathTracingScene::OnEnter(Context* context) {
  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{glm::vec3(0.286482, 0.708117, -1.065640)},
                          glm::quat(0.070520, {-0.005535, -0.994436, -0.078057}), {1, 1, 1}});
  context->SetCamera(camera_.get());

  // path tracing
  glm::ivec2 viewport_size = context->io().screen_size();
  std::vector<glm::vec4> canvas(viewport_size.x * viewport_size.y);
  for (glm::vec4& elem : canvas) {
    elem = engine::kBlack;
  }
  canvas_ = context->CreateTexture({viewport_size.x, viewport_size.y, canvas, GL_NEAREST, GL_NEAREST});

  RaytracingDebugCommon::LightPath light_path;
  light_path_ssbo_.Init(0, sizeof(light_path), &light_path);

  for (auto& p : conell_box_) {
    p.second.object.Init(context, p.first, p.first);
    p.second.object.SetTransform(p.second.transform);
    std::vector<engine::Primitive> model_primitives = p.second.object.GetPrimitives(context, p.second.primitive_index);
    for (const engine::Primitive& model_primitive : model_primitives) {
      primitives_.push_back(model_primitive);
    }
  }
  bvh_.Build(primitives_, {3, 12});

  //std::vector<engine::AABB> aabbs = sphere_.GetAABBs(context);
  //std::vector<engine::Primitive> primitives(aabbs.size());
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
  LinesObject bvh_lines;
  bvh_lines.SetMesh(LinesObject::Mesh(aabbs, glm::vec4(0, 0, 1, 1)));
  LinesShader lines_shader({}, context, &bvh_lines);
  bvh_lines.OnRender(context);

  engine::Ray pick_ray = camera_->GetPickRay(context->io().GetCursorPosSS());
  engine::RayBVHResult result = engine::RayBVH(pick_ray, bvh_, primitives_);
  if (result.hitted) {
    LinesObject triangle_lines;
    LinesObject::Mesh mesh(primitives_[result.primitive_index].triangle, engine::kBlack);
    triangle_lines.SetMesh(mesh);
    LinesShader lines_shader({}, context, &triangle_lines);
    triangle_lines.OnRender(context);
  }

  LinesObject ray_lines;
  ray_lines.SetMesh(LinesObject::Mesh({glm::vec4(pick_ray.origin, 1.0), glm::vec4(pick_ray.origin + pick_ray.dir, 1.0)},
                                      {glm::vec4(0, 0, 1, 1), glm::vec4(0, 0, 1, 1)}, GL_LINES));
  LinesShader ray_lines_shader({}, context, &ray_lines);
  ray_lines.OnRender(context);

  LOG(ERROR) << pick_ray.AsString();
  LinesShader({}, context, &coord_object_);
  coord_object_.OnRender(context);
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
