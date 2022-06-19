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

  // Cornell box
  if (1) {
    for (auto& p : cornell_box_) {
      p.second.object.Init(context, p.first, p.first);
      p.second.object.SetTransform(p.second.transform);
      p.second.object.GetPrimitives(context, &primitives_);
    }
    bvh_.Build(primitives_, {3, engine::BVH::Partition::kPos, 12});
  }

  // Sphere
  if (0) {
    sphere_.GetPrimitives(context, &primitives_);
    bvh_.Build(primitives_, {10, engine::BVH::Partition::kPos, 64});
  }

  bvh_ssbo_ = bvh_.AsSSBO(1);

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
  // Sphere
  if (0) {
    ColorShader({glm::vec4(1, 1, 1, 1)}, context, &sphere_);
    sphere_.OnRender(context);
  }

  // Cornell box
  if (1) {
    for (auto& p : cornell_box_) {
      ColorShader _({p.second.color}, context, &p.second.object);
      p.second.object.OnRender(context);
    }
  }

/*
  std::vector<engine::AABB> aabbs = bvh_.GetAABBs();
  LinesObject bvh_lines;
  bvh_lines.SetMesh(LinesObject::Mesh(aabbs));
  LinesShader lines_shader({}, context, &bvh_lines);
  bvh_lines.OnRender(context);
  */

  engine::Ray pick_ray = camera_->GetPickRay(context->io().GetCursorPosSS());
  engine::RayBVHResult result = engine::RayBVH(pick_ray, bvh_, primitives_);
  if (result.hitted) {
    LinesObject triangle_lines;
    LinesObject::Mesh mesh(primitives_.GetTriangle(result.primitive_index), engine::kRed);
    triangle_lines.SetMesh(mesh);
    LinesShader({}, context, &triangle_lines);
    triangle_lines.OnRender(context);

    LinesObject aabb_lines;
    LinesObject::Mesh aabb_mesh({result.aabb}, engine::kGreen);
    aabb_lines.SetMesh(aabb_mesh);
    LinesShader({}, context, &aabb_lines);
    aabb_lines.OnRender(context);
  }

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
