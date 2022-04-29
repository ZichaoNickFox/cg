#include "playground/scene/pbr_prefiltered_color_cubemap_generator.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/object/sphere.h"
#include "playground/scene/common.h"
#include "playground/texture_repo.h"
#include "playground/util.h"

void PbrPrefilteredColorCubemapGenerator::OnEnter(Context *context)
{
  camera_->mutable_transform()->SetTranslation(glm::vec3(2.97, 3.95, 6.76));
  camera_->mutable_transform()->SetRotation(glm::quat(0.89, -0.21, 0.38, 0.09));
  context->SetCamera(camera_);

  glEnable(GL_DEPTH_TEST);

  for (int face = 0; face < 6; ++face) {
    cubemap_cameras_[face].mutable_transform()->SetRotation(rotations_[face]);
    cubemap_cameras_[face].SetPerspectiveFov(90.0);
    cubemap_cameras_[face].SetAspect(1);
  }

  for (int level = 0; level < kMipmapMaxLevel; ++level) {
    engine::ColorFrameBuffer::Option option;
    option.clear_color = context->clear_color();
    option.mrt = 1;
    option.size = kMipmapLevel0Size * static_cast<float>(std::pow(0.5, level));
    color_frame_buffers_[level].Init(option);
  }
}

void PbrPrefilteredColorCubemapGenerator::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "PbrIrradianceCubemapGenerator");

  cube_.OnUpdate(context);
}

void PbrPrefilteredColorCubemapGenerator::OnRender(Context *context)
{
  for (int level = 0; level < kMipmapMaxLevel; ++level) {
    for (int face = 0; face < 6; ++face) {
      float roughness = std::pow(0.5, kMipmapMaxLevel - level - 1);
      color_frame_buffers_[level].Bind();
      PbrPrefilteredColorCubemapGeneratorShader({context->GetTexture("pbr_environment_cubemap"),
                                                &cubemap_cameras_[face], roughness}, context, &cube_);
      cube_.OnRender(context);
      color_frame_buffers_[level].Unbind();
      context->SaveCubemap(kCubemapNamePrefix + std::to_string(level), face,
                           color_frame_buffers_[level].GetColorTexture());
    }
  }
  exit(0);
}

void PbrPrefilteredColorCubemapGenerator::OnExit(Context *context)
{
  cube_.OnDestory(context);
}