#include "playground/scene/path_tracing_scene.h"

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
  light_path_ssbo_.Init(0, light_path);

  std::vector<AABB> aabbs;
  for (auto& p : conell_box_) {
    p.second.Init(p.first);
    std::vector<AABB> model_aabbs = p.second.GetAABBs(context);
    aabbs.insert(aabbs.end(), model_aabbs.begin(), model_aabbs.end());
  }
  bvh_.Build(aabbs, {5, 64});
  bvh_aabbs_ = bvh_.GetAABBs();

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
  for (auto& p : conell_box_) {
    p.second.OnRender(context);
  }
  LinesObject lines_object;
  LinesObject::Mesh lines_mesh(bvh_aabbs_, glm::vec4(1, 0, 0, 1));
  lines_object.SetMesh(lines_mesh);
  LinesShader lines_shader({}, context, &lines_object);
  lines_object.OnRender(context);
}

void PathTracingScene::PathTracing(Context* context) {
  PathTracingDemoShader({context->io().screen_size(), camera_.get(),
                    util::AsValueVector(sphere_map_), context->frame_stat().frame_num(), canvas_}, context);
  RaytracingDebugCommon(canvas_, context, light_path_ssbo_.GetData<RaytracingDebugCommon::LightPath>());
}
