#include "playground/scene/cubemap_2_irradiancemap_tool_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/object/sphere.h"
#include "playground/scene/common.h"
#include "playground/texture_repo.h"

void Cubemap2IrradiancemapToolScene::OnEnter(Context *context)
{
  camera_->mutable_transform()->SetTranslation(glm::vec3(2.97, 3.95, 6.76));
  camera_->mutable_transform()->SetRotation(glm::quat(0.89, -0.21, 0.38, 0.09));
  context->SetCamera(camera_);

  glEnable(GL_DEPTH_TEST);

  for (int i = 0; i < 6; ++i) {
    cubemap_cameras_[i].mutable_transform()->SetRotation(rotations_[i]);
    cubemap_cameras_[i].SetPerspectiveFov(90.0);
    cubemap_cameras_[i].SetAspect(1);
  }

  engine::ColorFrameBuffer::Option option;
  option.clear_color = context->clear_color();
  option.mrt = 1;
  option.size = glm::ivec2{512, 512};
  color_frame_buffer_.Init(option);
}

void Cubemap2IrradiancemapToolScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "Cubemap2IrradianceToolScene");

  cube_.OnUpdate(context);
}

void Cubemap2IrradiancemapToolScene::OnRender(Context *context)
{
  for (int i = 0; i < 6; ++i) {
    color_frame_buffer_.Bind();
    Cubemap2IrradiancemapShader({context->GetTexture("skybox2"), &cubemap_cameras_[i]}, context, &cube_);
    cube_.OnRender(context);
    color_frame_buffer_.Unbind();

    context->SaveTexture(name_[i], color_frame_buffer_.GetColorTexture());
  }
  exit(0);
}

void Cubemap2IrradiancemapToolScene::OnExit(Context *context)
{
  cube_.OnDestory(context);
}