#include "playground/physics_scene/angry_bunny_scene.h"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "renderer/framebuffer_attachment.h"
#include "renderer/inspector.h"
#include "renderer/transform.h"
#include "base/util.h"

using namespace renderer;
using namespace physics;

void AngryBunnyScene::OnEnter() {
  Material tiled;
  // tiled.texture_diffuse = ;
  material_repo_.Add("tiled", tiled);
  object_repo_.AddOrReplace(object_metas_);

  camera_->mutable_transform()->SetTranslation(glm::vec3(-0.390283, 0.308249, 0.490882));
  camera_->mutable_transform()->SetRotation(glm::quat(0.928992, {-0.082275, -0.359432, -0.031833}));

  light_repo_.Add({Light::kSpotLight, camera_->transform().translation()});
  object_repo_.MutableObject("light")->transform.SetTranslation(camera_->transform().translation());
}

void AngryBunnyScene::OnUpdate() {
}

void AngryBunnyScene::OnRender() {
  for (const Object& object : object_repo_.GetObjects()) {
    PhongShader({}, *this, object);
  }
}

void AngryBunnyScene::OnExit() {
}