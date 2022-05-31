#include "playground/scene/equirectangular_2_cubemap_tool.h"

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
constexpr char input[] = "tropical_equirectangular";
constexpr char output[] = "pbr_environment_tropical";

void Equirectangular2CubemapTool::OnEnter(Context *context)
{
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
  option.size = glm::ivec2{kEnvironmentCubemapSize, kEnvironmentCubemapSize};
  color_framebuffer_.Init(option);
}

void Equirectangular2CubemapTool::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "PbrEnvironmentCubemapGenerator");

  cube_.OnUpdate(context);
}

void Equirectangular2CubemapTool::OnRender(Context *context, int instance_num) {
  // TODO : why * 4 * 4 not * 4
  engine::CubemapData data(1, kEnvironmentCubemapSize * kEnvironmentCubemapSize * 4 * 4);
  for (int face = 0; face < 6; ++face) {
    color_framebuffer_.Bind();
    PbrEnvironmentCubemapGerneratorShader({context->GetTexture(input, true), &cubemap_cameras_[face]},
                                          context, &cube_);
    cube_.OnRender(context);
    color_framebuffer_.Unbind();

    data.UpdateData(face, 0, color_framebuffer_.GetColorTextureData(0));
  }
  engine::CreateCubemapParam param{1, kEnvironmentCubemapSize, kEnvironmentCubemapSize, &data};
  context->ResetCubemap(output, param);
  context->SaveCubemap(output);
  exit(0);
}

void Equirectangular2CubemapTool::OnExit(Context *context)
{
  cube_.OnDestory(context);
}