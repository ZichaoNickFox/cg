#include "playground/scene/pbr_irradiance_cubemap_generator.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/repo/texture_repo.h"
#include "engine/transform.h"
#include "playground/object/sphere.h"
#include "playground/scene/common.h"

constexpr int kLevel0Size = 512;
constexpr char input[] = "pbr_environment_tropical";
constexpr char output[] = "pbr_irradiance_tropical";

void PbrIrradianceCubemapGenerator::OnEnter(Context *context) {
  camera_->mutable_transform()->SetTranslation(glm::vec3(2.97, 3.95, 6.76));
  camera_->mutable_transform()->SetRotation(glm::quat(0.89, -0.21, 0.38, 0.09));
  context->SetCamera(camera_.get());

  glEnable_(GL_DEPTH_TEST);

  for (int i = 0; i < 6; ++i) {
    cubemap_cameras_[i].SetTransform(transforms_[i]);
    cubemap_cameras_[i].SetPerspectiveFov(90.0);
    cubemap_cameras_[i].SetAspect(1);
  }

  engine::ColorFramebuffer::Option option;
  option.clear_color = context->clear_color();
  option.mrt = 1;
  option.size = glm::ivec2{kLevel0Size, kLevel0Size};
  color_framebuffer_.Init(option);
}

void PbrIrradianceCubemapGenerator::OnUpdate(Context *context) {
  OnUpdateCommon _(context, "PbrIrradianceCubemapGenerator");

  cube_.OnUpdate(context);
}

void PbrIrradianceCubemapGenerator::OnRender(Context *context) {
  engine::CubemapData data(3, kLevel0Size * kLevel0Size * 4 * 4);
  for (int face = 0; face < 6; ++face) {
    color_framebuffer_.Bind();
    PbrIrradianceCubemapGeneratorShader({context->GetTexture(input),
                                        &cubemap_cameras_[face]}, context, &cube_);
    cube_.OnRender(context);
    color_framebuffer_.Unbind();

    *data.mutable_vector(face, 0) = color_framebuffer_.GetColorTextureData(0);
  }
  engine::CreateCubemapParam param{1, kLevel0Size, kLevel0Size, &data};
  context->ResetCubemap(output, param);
  context->SaveCubemap(output);
  exit(0);
}

void PbrIrradianceCubemapGenerator::OnExit(Context *context) {
  cube_.OnDestory(context);
}