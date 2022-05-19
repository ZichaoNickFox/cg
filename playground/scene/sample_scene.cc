#include "playground/scene/sample_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/scene/common.h"

void SampleScene::OnEnter(Context *context)
{
  camera_->mutable_transform()->SetTranslation(glm::vec3(2.97, 3.95, 6.76));
  camera_->mutable_transform()->SetRotation(glm::quat(0.95, -0.21, 0.18, 0.04));
  camera_->SetFarClip(200);
  context->SetCamera(camera_.get());

  glEnable_(GL_DEPTH_TEST);
}

void SampleScene::OnUpdate(Context *context) {
  OnUpdateCommon(context, "SampleScene");

  sphere_.OnUpdate(context);
}

void SampleScene::OnRender(Context *context)
{
  SampleShader({}, context, &sphere_);
  sphere_.OnRender(context);
}

void SampleScene::OnExit(Context *context)
{
  sphere_.OnDestory(context);
}