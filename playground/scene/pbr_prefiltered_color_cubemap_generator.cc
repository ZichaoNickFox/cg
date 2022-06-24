#include "playground/scene/pbr_prefiltered_color_cubemap_generator.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "renderer/repo/texture_repo.h"
#include "renderer/transform.h"
#include "renderer/util.h"
#include "playground/object/sphere_object.h"
#include "playground/scene/common.h"

std::string input = "pbr_environment_tropical";
std::string output = "pbr_prefiltered_color_tropical";

void PbrPrefilteredColorCubemapGenerator::OnEnter(Scene *context)
{
  camera_->mutable_transform()->SetTranslation(glm::vec3(2.97, 3.95, 6.76));
  context->SetCamera(camera_.get());

  glEnable_(GL_DEPTH_TEST);

  for (int face = 0; face < 6; ++face) {
    cubemap_cameras_[face].SetTransform(transforms_[face]);
    cubemap_cameras_[face].SetPerspectiveFov(90.0);
    cubemap_cameras_[face].SetAspect(1);
  }

  for (int level = 0; level < kMipmapMaxLevel; ++level) {
    renderer::ColorFramebuffer::Option option;
    option.clear_color = context->clear_color();
    option.mrt = 1;
    int size = kLevel0Size * static_cast<float>(std::pow(0.5, level));
    option.size = glm::ivec2(size, size);
    color_framebuffers_[level].Init(option);
  }
}

void PbrPrefilteredColorCubemapGenerator::OnUpdate(Scene *context)
{
  OnUpdateCommon _(context, "PbrPrefilteredColorCubemapGenerator");

  cube_.OnUpdate(context);
}

void PbrPrefilteredColorCubemapGenerator::OnRender(Scene *context, int instance_num)
{
  // TODO why * 4 * 4?
  renderer::CubemapData data(kMipmapMaxLevel, kLevel0Size * kLevel0Size * 4 * 4);
  
  for (int level = 0; level < kMipmapMaxLevel; ++level) {
    for (int face = 0; face < 6; ++face) {
      float roughness = std::pow(0.5, kMipmapMaxLevel - level - 1);
      color_framebuffers_[level].Bind();
      PbrPrefilteredColorCubemapGeneratorShader({context->GetTexture(input),
                                                &cubemap_cameras_[face], roughness}, context, &cube_);
      cube_.OnRender(context);
      color_framebuffers_[level].Unbind();

      data.UpdateData(face, level, color_framebuffers_[level].GetColorTextureData(0));
    }
  }
  renderer::CubemapParam param{kMipmapMaxLevel, kLevel0Size, kLevel0Size, &data};
  context->ResetCubemap(output, param);
  context->SaveCubemap(output);
  exit(0);
}

void PbrPrefilteredColorCubemapGenerator::OnExit(Scene *context)
{
  cube_.OnDestory(context);
}