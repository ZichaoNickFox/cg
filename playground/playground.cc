#include "playground/playground.h"

#include "imgui.h"

#include "playground/scene/AA_scene.h"
#include "playground/scene/AA_test_scene.h"
#include "playground/scene/deferred_shading_scene.h"
#include "playground/scene/forward_shading_scene.h"
#include "playground/scene/model_scene.h"
#include "playground/scene/mrt_scene.h"
#include "playground/scene/normal_scene.h"
#include "playground/scene/pbr_environment_cubemap_generator.h"
#include "playground/scene/pbr_BRDF_integration_map_generator.h"
#include "playground/scene/pbr_irradiance_cubemap_generator.h"
#include "playground/scene/pbr_prefiltered_color_cubemap_generator.h"
#include "playground/scene/pbr_scene.h"
#include "playground/scene/phong_scene.h"
#include "playground/scene/sample_scene.h"
#include "playground/scene/share_scene.h"
#include "playground/scene/skybox_scene.h"
#include "playground/scene/texture_lod_scene.h"

namespace {
const std::string kDefaultScene = "ModelScene";
}

void Playground::Init(const Context::Option& option) {
  InitScene();
  context_.Init(option);
}

void Playground::InitScene() {
  scene_map_.insert(std::make_pair("PhongScene", std::make_unique<PhongScene>()));
  scene_map_.insert(std::make_pair("ForwardShadingScene", std::make_unique<ForwardShadingScene>()));
  scene_map_.insert(std::make_pair("DeferredShadingScene", std::make_unique<DeferredShadingScene>()));
  scene_map_.insert(std::make_pair("SkyboxScene", std::make_unique<SkyboxScene>()));
  scene_map_.insert(std::make_pair("MrtScene", std::make_unique<MrtScene>()));
  scene_map_.insert(std::make_pair("AAScene", std::make_unique<AAScene>()));
  scene_map_.insert(std::make_pair("AATestScene", std::make_unique<AATestScene>()));
  scene_map_.insert(std::make_pair("ModelScene", std::make_unique<ModelScene>()));
  scene_map_.insert(std::make_pair("PbrScene", std::make_unique<PbrScene>()));
  scene_map_.insert(std::make_pair("NormalScene", std::make_unique<NormalScene>()));
  scene_map_.insert(std::make_pair("ShareScene", std::make_unique<ShareScene>()));
  scene_map_.insert(std::make_pair("PbrEnvironmentCubemapGenerator", std::make_unique<PbrEnvironmentCubemapGenerator>()));
  scene_map_.insert(std::make_pair("PbrIrradianceCubemapGenerator", std::make_unique<PbrIrradianceCubemapGenerator>()));
  scene_map_.insert(std::make_pair("SampleScene", std::make_unique<SampleScene>()));
  scene_map_.insert(std::make_pair("PbrPrefilteredColorCubemapGenerator", std::make_unique<PbrPrefilteredColorCubemapGenerator>()));
  scene_map_.insert(std::make_pair("PbrBRDFIntegrationMapGenerator", std::make_unique<PbrBRDFIntegrationMapGenerator>()));
  scene_map_.insert(std::make_pair("TextureLodScene", std::make_unique<TextureLodScene>()));
}

void Playground::BeginFrame() {
  CGLOG(ERROR, false) << "begin--------begin---------begin";
  frame_start_time_ = std::chrono::high_resolution_clock::now();
}

void Playground::Update() {
  SwitchScene(kDefaultScene);

  const std::unique_ptr<Scene>& scene = scene_map_[current_scene_];
  scene->OnUpdate(&context_);
}

void Playground::Render() {
  const std::unique_ptr<Scene>& scene = scene_map_[current_scene_];
  scene->OnRender(&context_);
}

void Playground::EndFrame() {
  context_.mutable_io()->ClearKeyInput();
  util::Time frame_end_time = std::chrono::high_resolution_clock::now();
  int64_t frame_interval_millisecond = util::DurationMillisecond(frame_start_time_, frame_end_time);
  context_.SetFrameInternal(frame_interval_millisecond);
  CGLOG(ERROR, false) << "end--------end---------end";
}

void Playground::SwitchScene(const std::string& next_scene) {
  if (next_scene == current_scene_) {
    return;
  }
  if (current_scene_ != "") {
    CGCHECK_NOTNULL(scene_map_[current_scene_])->OnExit(&context_);
  }

  CGCHECK_NOTNULL(scene_map_[next_scene])->OnEnter(&context_);
  current_scene_ = next_scene;
}