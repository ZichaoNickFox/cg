#include "playground/playground.h"

#include <glm/glm.hpp>
#include "imgui.h"

#include "playground/geometry_scene/fitting_scene.h"
#include "playground/geometry_scene/implot_demo_scene.h"
#include "playground/physics_scene/angry_bunny_scene.h"
//#include "playground/renderer_scene/equirectangular_2_cubemap_tool.h"
//#include "playground/renderer_scene/forward_shading_scene.h"
#include "playground/renderer_scene/geometry_scene.h"
//#include "playground/renderer_scene/instance_scene.h"
#include "playground/renderer_scene/model_scene.h"
//#include "playground/renderer_scene/mrt_scene.h"
//#include "playground/renderer_scene/normal_scene.h"
#include "playground/renderer_scene/path_tracing_scene.h"
#include "playground/renderer_scene/path_tracing_geometry_scene.h"
//#include "playground/renderer_scene/pbr_BRDF_integration_map_generator.h"
//#include "playground/renderer_scene/pbr_irradiance_cubemap_generator.h"
//#include "playground/renderer_scene/pbr_prefiltered_color_cubemap_generator.h"
//#include "playground/renderer_scene/pbr_scene.h"
//#include "playground/renderer_scene/random_test_scene.h"
#include "playground/renderer_scene/ray_tracing_scene.h"
#include "playground/renderer_scene/rtrt_scene.h"
#include "playground/renderer_scene/sample_scene.h"
//#include "playground/renderer_scene/shadow_scene.h"
//#include "playground/renderer_scene/share_scene.h"
//#include "playground/renderer_scene/skybox_scene.h"
#include "playground/renderer_scene/ssao_scene.h"
//#include "playground/renderer_scene/texture_lod_scene.h"

const std::string kConfigPath = "playground/config.pb.txt";
const std::string kDefaultSceneName = "FittingScene";
const std::unordered_map<std::string, std::function<cg::Scene*()>> kFactory = {
  // {"ForwardShadingScene", [] () { return new ForwardShadingScene; }},
  // {"DeferredShadingScene", [] () { return new DeferredShadingScene; }},
  // {"SkyboxScene", [] () { return new SkyboxScene; }},
  // {"MrtScene", [] () { return new MrtScene; }},
  // {"AAScene", [] () { return new AAScene; }},
  // {"AATestScene", [] () { return new AATestScene; }},
  // {"PbrScene", [] () { return new PbrScene; }},
  // {"NormalScene", [] () { return new NormalScene; }},
  // {"ShareScene", [] () { return new ShareScene; }},
  // {"Equirectangular2CubemapTool", [] () { return new Equirectangular2CubemapTool; }},
  // {"PbrIrradianceCubemapGenerator", [] () { return new PbrIrradianceCubemapGenerator; }},
  // {"SampleTestScene", [] () { return new SampleTestScene; }};
  // {"PbrPrefilteredColorCubemapGenerator", [] () { return new PbrPrefilteredColorCubemapGenerator; }};
  // {"PbrBRDFIntegrationMapGenerator", [] () { return new PbrBRDFIntegrationMapGenerator; }};
  // {"TextureLodScene", [] () { return new TextureLodScene; }},
  // {"SSAOScene", [] () { return new SSAOScene; }},
  // {"ShadowScene", [] () { return new ShadowScene; }},
  // {"DepthScene", [] () { return new DepthScene; }},
  // {"InstanceScene", [] () { return new InstanceScene; }},
  // {"RandomTestScene", [] () { return new RandomTestScene; }},
  {"GeometryScene", [] () { return new GeometryScene(); }},
  {"ModelScene", [] () { return new ModelScene(); }},
  {"RayTracingScene", [] () { return new RayTracingScene(); }},
  {"PathTracingGeometryScene", [] () { return new PathTracingGeometryScene(); }},
  {"PathTracingScene", [] () { return new PathTracingScene(); }},
  {"SampleScene", [] () { return new SampleScene(); }},
  {"RTRTScene", [] () { return new RTRTScene(); }},
  {"SSAOScene", [] () { return new SSAOScene(); }},
  {"AngryBunnyScene", [] () { return new AngryBunnyScene(); }},
  {"FittingScene", [] () { return new FittingScene(); }},
  {"ImPlotDemoScene", [] () { return new ImPlotDemoScene(); }},
};

Playground::Playground() {
  config_.Init(kConfigPath);
}

void Playground::BeginFrame() {
  if (current_scene_name_ == "") {
    current_scene_name_ = kDefaultSceneName;
    CGCHECK(kFactory.count(kDefaultSceneName) > 0) << kDefaultSceneName;
    current_scene_ = kFactory.at(current_scene_name_)();
    current_scene_->Enter(current_scene_name_, &config_, &io_, &frame_stat_);
  }
  frame_start_time_ = std::chrono::high_resolution_clock::now();
}

void Playground::Update() {
  current_scene_->Update();
}

void Playground::Render() {
  current_scene_->Render();
}

void Playground::EndFrame() {
  io_.ClearKeyInput();
  util::Time frame_end_time = std::chrono::high_resolution_clock::now();
  int64_t frame_interval_millisecond = util::DurationMillisecond(frame_start_time_, frame_end_time);
  frame_stat_.OnFrame(frame_interval_millisecond);
}

void Playground::Destoy() {
  if (current_scene_) {
    delete current_scene_;
  }
}
