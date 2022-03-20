#include "framework.h"

#include "imgui.h"

#include "engine/util.h"
#include "playground/scene/imgui_demo_scene.h"
#include "playground/scene/gallery_scene.h"
#include "playground/scene/test_scene.h"
#include "playground/scene/triangle_scene.h"
#include "playground/scene/cube_world_scene.h"
#include "playground/scene/phong_scene.h"
#include "playground/scene/shadow_scene.h"
#include "playground/scene/skybox_scene.h"
#include "playground/scene/shadow_map_scene.h"

namespace {
const SceneType kDefaultSceneType = SceneType::ShadowMap;
}

void Framework::Init(const std::string& config_path) {
  InitScene();
  InitContext(config_path);
}

void Framework::InitContext(const std::string& config_path) {
  std::string content;
  engine::util::ReadFileToString(config_path, &content);

  Config config;
  engine::util::ParseFromString(content, &config);
  context_.Init(config);

  SwitchScene(kDefaultSceneType, true);
}

void Framework::InitScene() {
  scene_map_.insert(std::make_pair(Gallery, std::make_unique<GalleryScene>()));
  scene_map_.insert(std::make_pair(Test, std::make_unique<TestScene>()));
  scene_map_.insert(std::make_pair(ImGuiDemo, std::make_unique<ImGuiDemoScene>()));
  scene_map_.insert(std::make_pair(Triangle, std::make_unique<TriangleScene>()));
  scene_map_.insert(std::make_pair(CubeWorld, std::make_unique<CubeWorldScene>()));
  scene_map_.insert(std::make_pair(Phong, std::make_unique<PhongScene>()));
  scene_map_.insert(std::make_pair(Shadow, std::make_unique<ShadowScene>()));
  scene_map_.insert(std::make_pair(Skybox, std::make_unique<SkyboxScene>()));
  scene_map_.insert(std::make_pair(ShadowMap, std::make_unique<ShadowMapScene>()));

  if (SceneType_ARRAYSIZE != scene_map_.size()) {
    CHECK(false) << "Add scene here";
  }
}

void Framework::BeginFrame() {
}

void Framework::Update() {
  if (context_.current_scene_type() != context_.next_scene_type()) {
    SwitchScene(context_.next_scene_type());
  }
}

void Framework::Gui() {
  const std::unique_ptr<Scene>& scene = scene_map_[context_.current_scene_type()];
  scene->OnGui(&context_);
}

void Framework::Render() {
  const std::unique_ptr<Scene>& scene = scene_map_[context_.current_scene_type()];
  scene->OnUpdate(&context_);
  scene->OnRender(&context_);
}

void Framework::EndFrame() {
  context_.mutable_io()->ClearKeyInput();
}

void Framework::SwitchScene(SceneType scene_type, bool ignore_current_scene) {
  if (!ignore_current_scene) {
    const std::unique_ptr<Scene>& current_scene = scene_map_[context_.current_scene_type()];
    current_scene->OnExit(&context_);
  }

  context_.SetCurrentSceneType(scene_type);
  context_.SetNextSceneType(scene_type);

  const std::unique_ptr<Scene>& next_scene = scene_map_[scene_type];
  next_scene->OnEnter(&context_);
}