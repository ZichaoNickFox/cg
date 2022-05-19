#include "playground/scene/skybox_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/math.h"
#include "engine/transform.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void SkyboxScene::OnEnter(Context *context)
{
  camera_->mutable_transform()->SetTranslation(glm::vec3(-1.0, 1.5, 1.1));
  camera_->mutable_transform()->SetRotation(glm::angleAxis(float(M_PI) / 2, glm::vec3(0, 1, 0)));
  camera_->SetFarClip(200);
  context->SetCamera(camera_.get());

  skybox_.mutable_material()->SetShader(context->GetShader("skybox"));
  skybox_.mutable_material()->SetTexture("texture0", context->GetTexture("skybox"));
  skybox_.mutable_transform()->SetScale(glm::vec3(100, 100, 100));

  glEnable_(GL_DEPTH_TEST);
}

void SkyboxScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "SkyboxScene");
  coord_.OnUpdate(context);
  skybox_.OnUpdate(context);
}

void SkyboxScene::OnRender(Context *context)
{
  CubemapShader({context->GetTexture("pbr_environment_tropical")}, context, &skybox_);
  skybox_.OnRender(context);

  LinesShader({}, context, &coord_);
  coord_.OnRender(context);
}

void SkyboxScene::OnExit(Context *context)
{
  coord_.OnDestory(context);
  skybox_.OnDestory(context);
}