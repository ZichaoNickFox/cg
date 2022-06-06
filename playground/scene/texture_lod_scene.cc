#include "playground/scene/texture_lod_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/repo/texture_repo.h"
#include "engine/transform.h"
#include "playground/object/sphere_object.h"
#include "playground/scene/common.h"

constexpr int kLevel0Size = 512;

void TextureLodScene::OnEnter(Context *context) {
  context->SetCamera(camera_.get());
  camera_->mutable_transform()->SetTranslation(glm::vec3(0, 0, 5));

  glEnable_(GL_DEPTH_TEST);

  engine::ColorFramebuffer::Option option;
  option.clear_color = context->clear_color();
  option.mrt = 1;
  option.size = glm::ivec2{kLevel0Size, kLevel0Size};
  color_framebuffer_.Init(option);

  InitTexture2DLod(context);
  InitCubemapLod(context);
}

void TextureLodScene::InitTexture2DLod(Context *context) {
  // TODO : why a 512 * 512 rgba8 texture must use 512 * 512 * 4 * 4 size?
  engine::Texture2DData data(3, kLevel0Size * kLevel0Size * 4 * 4);
  // Red
  for (int i = 0; i < kLevel0Size * kLevel0Size * 4; i += 4) {
    data.mutable_data(0)[i] = 255;
    data.mutable_data(0)[i + 3] = 255;
  }
  // Green
  for (int i = 0; i < (kLevel0Size >> 1) * (kLevel0Size >> 1) * 4; i += 4) {
    data.mutable_data(1)[i + 1] = 255;
    data.mutable_data(1)[i + 3] = 255;
  }
  // Blue
  for (int i = 0; i < (kLevel0Size >> 2) * (kLevel0Size >> 2) * 4; i += 4) {
    data.mutable_data(2)[i + 2] = 255;
    data.mutable_data(2)[i + 3] = 255;
  }
  context->ResetTexture2D("texture2d_lod_rgb", {3, kLevel0Size, kLevel0Size, &data});
  context->SaveTexture2D("texture2d_lod_rgb");
  lod_texture2d_ = context->GetTexture("texture2d_lod_rgb");
}

void TextureLodScene::InitCubemapLod(Context *context) {
  // TODO : why a 512 * 512 rgba8 texture must use 512 * 512 * 4 * 4 size?
  engine::CubemapData data(3, kLevel0Size * kLevel0Size * 4 * 4);
  for (int texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset) {
    // Red
    for (int i = 0; i < kLevel0Size * kLevel0Size * 4; i += 4) {
      data.mutable_data(texture_unit_offset, 0)[i] = 255;
      data.mutable_data(texture_unit_offset, 0)[i + 3] = 255;
    }
    // Green
    for (int i = 0; i < (kLevel0Size >> 1) * (kLevel0Size >> 1) * 4; i += 4) {
      data.mutable_data(texture_unit_offset, 1)[i + 1] = 255;
      data.mutable_data(texture_unit_offset, 1)[i + 3] = 255;
    }
    // Blue
    for (int i = 0; i < (kLevel0Size >> 2) * (kLevel0Size >> 2) * 4; i += 4) {
      data.mutable_data(texture_unit_offset, 2)[i + 2] = 255;
      data.mutable_data(texture_unit_offset, 2)[i + 3] = 255;
    }
  }
  context->ResetCubemap("cubemap_lod_rgb", {3, kLevel0Size, kLevel0Size, &data});
  context->SaveCubemap("cubemap_lod_rgb");
  lod_cubemap_ = context->GetTexture("cubemap_lod_rgb");
}

void TextureLodScene::OnUpdate(Context *context) {
  OnUpdateCommon _(context, "TextureLodScene");
}

void TextureLodScene::OnRender(Context *context) {
  CubeObject cube1;
  cube1.mutable_transform()->SetTranslation(glm::vec3(-1.5, 0, 0));
  Texture2DLodShader({lod_texture2d_, context->camera().transform().translation()}, context, &cube1);
  cube1.OnRender(context);

  CubeObject cube2;
  cube1.mutable_transform()->SetTranslation(glm::vec3(1.5, 0, 0));
  CubemapLodShader({lod_cubemap_, context->camera().transform().translation()}, context, &cube2);
  cube2.OnRender(context);
}

void TextureLodScene::OnExit(Context *context) {
}