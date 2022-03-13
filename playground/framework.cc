#include "framework.h"

#include "imgui.h"

#include "engine/file_util.h"
#include "engine/proto_util.h"
#include "playground/scene/imgui_demo_scene.h"
#include "playground/scene/gallery_scene.h"
#include "playground/scene/test_scene.h"
#include "playground/scene/triangle_scene.h"
#include "playground/scene/cube_world_scene.h"
#include "playground/scene/phong_scene.h"

namespace {
const SceneType kDefaultSceneType = SceneType::Phong;
}

void Framework::Init(const std::string& config_path) {
  InitScene();
  InitContext(config_path);
}

void Framework::InitContext(const std::string& config_path) {
  std::string content;
  engine::file_util::ReadFileToString(config_path, &content);

  Config config;
  engine::proto_util::ParseFromString(content, &config);
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

  if (SceneType_ARRAYSIZE != scene_map_.size()) {
    CHECK(false) << "Add scene here";
  }
}

void Framework::BeginFrame(const IoInput& io_input) {
  for (const std::string& key_input : io_input.key_input) {
    context_.mutable_io()->FeedKeyInput(key_input);
  }
  context_.mutable_io()->FeedCursorPos(io_input.cursor_x, io_input.cursor_y);
  context_.mutable_io()->FeedButtonInput(io_input.left_button_pressed, io_input.right_button_pressed);
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