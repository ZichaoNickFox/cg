#include "renderer/scene.h"

#include "renderer/debug.h"
#include "renderer/definition.h"
#include "renderer/scene_common.h"
#include "renderer/util.h"

namespace renderer {
void Scene::Enter(const std::string& name, Config* config, Io* io, FrameStat* frame_stat) {
  name_ = name;

  config_ = config;
  io_ = io;
  frame_stat_ = frame_stat;

  shader_program_repo_.Init(*config);

  glEnable_(GL_DEPTH_TEST);

  OnEnter();
}

void Scene::Update() {
  OnUpdateCommon(this, name_);

  material_repo_.UpdateSSBO();
  light_repo_.UpdateSSBO();
  bvh_.UpdateSSBO();
  primitive_repo_.UpdateSSBO();

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
} // namespace scene
