#include "playground/playground.h"

#include "imgui.h"

#include "playground/scene/imgui_demo_scene.h"
#include "playground/scene/gallery_scene.h"
#include "playground/scene/test_scene.h"
#include "playground/scene/triangle_scene.h"
#include "playground/scene/cube_world_scene.h"
#include "playground/scene/phong_scene.h"
#include "playground/scene/shadow_scene.h"
#include "playground/scene/skybox_scene.h"
#include "playground/scene/shadow_map_scene.h"
#include "playground/util.h"

namespace {
const std::string kDefaultScene = "ShadowMapScene";
}

void Playground::Init(const std::string& config_path) {
  InitScene();
  InitContext(config_path);
}

void Playground::InitContext(const std::string& config_path) {
  std::string content;
  util::ReadFileToString(config_path, &content);

  Config config;
  util::ParseFromString(content, &config);
  context_.Init(config);

  SwitchScene(kDefaultScene, true);
}

void Playground::InitScene() {
  scene_map_.insert(std::make_pair("GalleryScene", std::make_unique<GalleryScene>()));
  scene_map_.insert(std::make_pair("TestScene", std::make_unique<TestScene>()));
  scene_map_.insert(std::make_pair("ImGuiDemoScene", std::make_unique<ImGuiDemoScene>()));
  scene_map_.insert(std::make_pair("TriangleScene", std::make_unique<TriangleScene>()));
  scene_map_.insert(std::make_pair("CubeWorldScene", std::make_unique<CubeWorldScene>()));
  scene_map_.insert(std::make_pair("PhongScene", std::make_unique<PhongScene>()));
  scene_map_.insert(std::make_pair("ShadowScene", std::make_unique<ShadowScene>()));
  scene_map_.insert(std::make_pair("SkyboxScene", std::make_unique<SkyboxScene>()));
  scene_map_.insert(std::make_pair("ShadowMapScene", std::make_unique<ShadowMapScene>()));
}

void Playground::BeginFrame() {
}

void Playground::Update() {
  if (context_.current_scene() != context_.next_scene()) {
    SwitchScene(context_.next_scene());
  }
  const std::unique_ptr<Scene>& scene = scene_map_[context_.current_scene()];
  scene->OnUpdate(&context_);
}

void Playground::Gui() {
  const std::unique_ptr<Scene>& scene = scene_map_[context_.current_scene()];
  scene->OnGui(&context_);
}

void Playground::Render() {
  const std::unique_ptr<Scene>& scene = scene_map_[context_.current_scene()];
  scene->OnRender(&context_);
}

void Playground::EndFrame() {
  context_.mutable_io()->ClearKeyInput();
}

void Playground::SwitchScene(const std::string& scene, bool ignore_current_scene) {
  if (!ignore_current_scene) {
    const std::unique_ptr<Scene>& current_scene = scene_map_[context_.current_scene()];
    current_scene->OnExit(&context_);
  }

  context_.SetCurrentScene(scene);
  context_.SetNextScene(scene);

  const std::unique_ptr<Scene>& next_scene = scene_map_[scene];
  next_scene->OnEnter(&context_);
}