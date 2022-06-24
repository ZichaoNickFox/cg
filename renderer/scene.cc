#include "renderer/scene.h"

#include "renderer/debug.h"
#include "renderer/definition.h"
#include "renderer/scene_common.h"
#include "renderer/util.h"

namespace renderer {
void Scene::Init(const Option& option, const std::string& scene_name) {
  config_.Init(option.config_path);
  shader_repo_.Init(config_);
  option_ = option;
  name_ = scene_name;
}

void Scene::Enter() {
  glEnable_(GL_DEPTH_TEST);

  OnEnter();

  material_repo_.BindSSBO(SSBO_MATERIAL_REPO);
  light_repo_.BindSSBO(SSBO_LIGHT_REPO);
}

void Scene::Update() {
  OnUpdateCommon(this, name_);
  
  OnUpdate();
}

void Scene::Render() {
  OnRender();

  // OnRenderCommon(this);
}

void Scene::Exit() {
  OnExit();
}

const Camera& Scene::camera() const {
  CGCHECK(camera_) << "camera must be nullptr";
  return *camera_.get();
}

Camera* Scene::mutable_camera() {
  return camera_.get();
}

void Scene::StatFrame(int last_frame_interval) {
  frame_stat_.OnFrame(last_frame_interval);
}

std::function<void(const std::string& content)> Scene::set_clipboard_string_func() {
  return option_.set_clipboard_string_func;
}
} // namespace scene
