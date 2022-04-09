#include "playground/playground.h"

#include "imgui.h"

#include "playground/scene/AA_scene.h"
#include "playground/scene/AA_test_scene.h"
#include "playground/scene/cube_world_scene.h"
#include "playground/scene/deferred_shading_scene.h"
#include "playground/scene/forward_shading_scene.h"
#include "playground/scene/gallery_scene.h"
#include "playground/scene/model_scene.h"
#include "playground/scene/mrt_scene.h"
#include "playground/scene/pbr_scene.h"
#include "playground/scene/phong_scene.h"
#include "playground/scene/shadow_map_scene.h"
#include "playground/scene/skybox_scene.h"
#include "playground/scene/triangle_scene.h"

namespace {
const std::string kDefaultScene = "DeferredShadingScene";
}

void Playground::Init(const Context::Option& option) {
  InitScene();
  context_.Init(option);
  SwitchScene(kDefaultScene, true);
}

void Playground::InitScene() {
  scene_map_.insert(std::make_pair("GalleryScene", std::make_unique<GalleryScene>()));
  scene_map_.insert(std::make_pair("TriangleScene", std::make_unique<TriangleScene>()));
  scene_map_.insert(std::make_pair("CubeWorldScene", std::make_unique<CubeWorldScene>()));
  scene_map_.insert(std::make_pair("PhongScene", std::make_unique<PhongScene>()));
  scene_map_.insert(std::make_pair("ForwardShadingScene", std::make_unique<ForwardShadingScene>()));
  scene_map_.insert(std::make_pair("DeferredShadingScene", std::make_unique<DeferredShadingScene>()));
  scene_map_.insert(std::make_pair("SkyboxScene", std::make_unique<SkyboxScene>()));
  scene_map_.insert(std::make_pair("ShadowMapScene", std::make_unique<ShadowMapScene>()));
  scene_map_.insert(std::make_pair("MrtScene", std::make_unique<MrtScene>()));
  scene_map_.insert(std::make_pair("AAScene", std::make_unique<AAScene>()));
  scene_map_.insert(std::make_pair("AATestScene", std::make_unique<AATestScene>()));
  scene_map_.insert(std::make_pair("ModelScene", std::make_unique<ModelScene>()));
  scene_map_.insert(std::make_pair("PbrScene", std::make_unique<PbrScene>()));
}

void Playground::BeginFrame() {
  CGLOG(ERROR, false) << "begin--------begin---------begin";
  frame_start_time_ = std::chrono::high_resolution_clock::now();
}

void Playground::Update() {
  if (context_.current_scene() != context_.next_scene()) {
    SwitchScene(context_.next_scene());
  }
  const std::unique_ptr<Scene>& scene = scene_map_[context_.current_scene()];
  scene->OnUpdate(&context_);
}

void Playground::Render() {
  const std::unique_ptr<Scene>& scene = scene_map_[context_.current_scene()];
  scene->OnRender(&context_);
}

void Playground::EndFrame() {
  context_.mutable_io()->ClearKeyInput();
  util::Time frame_end_time = std::chrono::high_resolution_clock::now();
  int64_t frame_interval_millisecond = util::DurationMillisecond(frame_start_time_, frame_end_time);
  context_.SetFrameInternal(frame_interval_millisecond);
  CGLOG(ERROR, false) << "end--------end---------end";
}

void Playground::SwitchScene(const std::string& scene, bool ignore_current_scene) {
  CGCHECK(scene_map_.count(scene) > 0) << " Cannot find scene : " << scene;
  if (!ignore_current_scene) {
    const std::unique_ptr<Scene>& current_scene = scene_map_[context_.current_scene()];
    current_scene->OnExit(&context_);
  }

  context_.SetCurrentScene(scene);
  context_.SetNextScene(scene);

  const std::unique_ptr<Scene>& next_scene = scene_map_[scene];
  next_scene->OnEnter(&context_);
}