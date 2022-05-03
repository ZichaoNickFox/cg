#include "playground/scene/pbr_environment_cubemap_generator.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/repo/texture_repo.h"
#include "engine/transform.h"
#include "playground/object/sphere.h"
#include "playground/scene/common.h"

constexpr int kEnvironmentCubemapSize = 2048;

void PbrEnvironmentCubemapGenerator::OnEnter(Context *context)
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

  engine::ColorFramebuffer::Option option;
  option.clear_color = context->clear_color();
  option.mrt = 1;
  option.size = glm::ivec2{kEnvironmentCubemapSize, kEnvironmentCubemapSize};
  color_framebuffer_.Init(option);
}

void PbrEnvironmentCubemapGenerator::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "PbrEnvironmentCubemapGenerator");

  cube_.OnUpdate(context);
}

void PbrEnvironmentCubemapGenerator::OnRender(Context *context) {
  // TODO : why * 4 * 4 not * 4
  engine::CubemapData data(1, kEnvironmentCubemapSize * kEnvironmentCubemapSize * 4 * 4);
  for (int face = 0; face < 6; ++face) {
    color_framebuffer_.Bind();
    PbrEnvironmentCubemapGerneratorShader({context->GetTexture("tropical_equirectangular", true), &cubemap_cameras_[face]},
                                          context, &cube_);
    cube_.OnRender(context);
    color_framebuffer_.Unbind();

    *data.mutable_vector(face, 0) = color_framebuffer_.GetColorTextureData(0);
  }
  engine::CreateCubemapParam param{1, kEnvironmentCubemapSize, kEnvironmentCubemapSize, &data};
  context->ResetCubemap("pbr_environment_cubemap", param);
  context->SaveCubemap("pbr_environment_cubemap");
  exit(0);
}

void PbrEnvironmentCubemapGenerator::OnExit(Context *context)
{
  cube_.OnDestory(context);
}