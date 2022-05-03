#include "playground/scene/pbr_prefiltered_color_cubemap_generator.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/repo/texture_repo.h"
#include "engine/transform.h"
#include "engine/util.h"
#include "playground/object/sphere.h"
#include "playground/scene/common.h"

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
    int size = kLevel0Size * static_cast<float>(std::pow(0.5, level));
    option.size = glm::ivec2(size, size);
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
  // TODO why * 4 * 4?
  engine::CubemapData data(kMipmapMaxLevel, kLevel0Size * kLevel0Size * 4 * 4);
  for (int level = 0; level < kMipmapMaxLevel; ++level) {
    for (int face = 0; face < 6; ++face) {
      float roughness = std::pow(0.5, kMipmapMaxLevel - level - 1);
      color_frame_buffers_[level].Bind();
      PbrPrefilteredColorCubemapGeneratorShader({context->GetTexture("pbr_environment_cubemap"),
                                                &cubemap_cameras_[face], roughness}, context, &cube_);
      cube_.OnRender(context);
      color_frame_buffers_[level].Unbind();

      *data.mutable_vector(face, level) = color_frame_buffers_[level].GetColorTextureData(0);
    }
  }
  engine::ResetCubemapParam param{kMipmapMaxLevel, kLevel0Size, kLevel0Size, &data};
  context->ResetCubemap("pbr_prefiltered_color_cubemap", param);
  context->SaveCubemap("pbr_prefiltered_color_cubemap");
  exit(0);
}

void PbrPrefilteredColorCubemapGenerator::OnExit(Context *context)
{
  cube_.OnDestory(context);
}