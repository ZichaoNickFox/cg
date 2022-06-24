#include "playground/playground.h"

#include "imgui.h"

//#include "playground/scene/AA_scene.h"
//#include "playground/scene/AA_test_scene.h"
//#include "playground/scene/deferred_shading_scene.h"
//#include "playground/scene/depth_scene.h"
//#include "playground/scene/forward_shading_scene.h"
#include "playground/scene/model_scene.h"
//#include "playground/scene/mrt_scene.h"
//#include "playground/scene/normal_scene.h"
//#include "playground/scene/equirectangular_2_cubemap_tool.h"
//#include "playground/scene/pbr_BRDF_integration_map_generator.h"
//#include "playground/scene/pbr_irradiance_cubemap_generator.h"
//#include "playground/scene/pbr_prefiltered_color_cubemap_generator.h"
#include "playground/scene/ray_tracing_scene.h"
#include "playground/scene/path_tracing_scene.h"
//#include "playground/scene/pbr_scene.h"
//#include "playground/scene/random_test_scene.h"
//#include "playground/scene/geometry_scene.h"
//#include "playground/scene/sample_test_scene.h"
//#include "playground/scene/shadow_scene.h"
//#include "playground/scene/share_scene.h"
//#include "playground/scene/skybox_scene.h"
//#include "playground/scene/SSAO_scene.h"
//#include "playground/scene/texture_lod_scene.h"
//#include "playground/scene/instance_scene.h"

const std::string kDefaultSceneName = "ModelScene";
const std::unordered_map<std::string, std::function<renderer::Scene*()>> kFactory = {
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
  // {"GeometryScene", [] () { return new GeometryScene; }},
  {"ModelScene", [] () { return new ModelScene; }},
  {"RayTracingScene", [] () { return new RayTracingScene; }},
  {"PathTracingScene", [] () { return new PathTracingScene; }},
};

void Playground::Init(const renderer::Scene::Option& option) {
  current_scene_ = kFactory.at(kDefaultSceneName)();
  current_scene_->Init(option, kDefaultSceneName);
  current_scene_->Enter();
}

void Playground::BeginFrame() {
  frame_start_time_ = std::chrono::high_resolution_clock::now();
}

void Playground::Update() {
  current_scene_->Update();
}

void Playground::Render() {
  current_scene_->Render();
}

void Playground::EndFrame() {
  current_scene_->mutable_io()->ClearKeyInput();
  util::Time frame_end_time = std::chrono::high_resolution_clock::now();
  int64_t frame_interval_millisecond = util::DurationMillisecond(frame_start_time_, frame_end_time);
  current_scene_->StatFrame(frame_interval_millisecond);
}

void Playground::Destoy() {
  if (current_scene_) {
    delete current_scene_;
  }
}
