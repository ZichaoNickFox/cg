#include "playground/scene/texture_lod_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/object/sphere.h"
#include "playground/scene/common.h"
#include "playground/texture_repo.h"

void TextureLodScene::OnEnter(Context *context) {
  context->SetCamera(camera_);
  camera_->mutable_transform()->SetTranslation(glm::vec3(0, 0, 5));

  glEnable(GL_DEPTH_TEST);

  engine::ColorFrameBuffer::Option option;
  option.clear_color = context->clear_color();
  option.mrt = 1;
  option.size = glm::ivec2{512, 512};
  color_frame_buffer_.Init(option);

  InitTexture2DLod(context);
  InitCubemapLod(context);
}

void TextureLodScene::InitTexture2DLod(Context *context) {
  // Red
  // TODO : why a 512 * 512 rgba8 texture must use 512 * 512 * 4 * 4 size?
  int lod0_size = 512;
  std::vector<GLubyte> data_red(lod0_size * lod0_size * 4 * 4);
  for (int i = 0; i < lod0_size * lod0_size * 4; i += 4) {
    data_red.data()[i] = 255;
    data_red.data()[i + 3] = 255;
  }
  texture::CreateTexture2DParam param_red{1, lod0_size, lod0_size, {data_red.data()}};
  context->CreateTexture2D("red", param_red);
  lod_texture2d_level_0_ = context->GetTexture("red");

  // Green
  int lod1_size = 256;
  std::vector<GLubyte> data_green(lod1_size * lod1_size * 4 * 4);
  for (int i = 0; i < lod1_size * lod1_size * 4; i += 4) {
    data_green.data()[i + 1] = 255;
    data_green.data()[i + 3] = 255;
  }
  texture::CreateTexture2DParam param_green{1, lod1_size, lod1_size, {data_red.data()}};
  context->CreateTexture2D("green", param_green);
  lod_texture2d_level_1_ = context->GetTexture("green");

  // Blue
  int lod2_size = 128;
  std::vector<GLubyte> data_blue(lod2_size * lod2_size * 4 * 4);
  for (int i = 0; i < lod2_size * lod2_size * 4; i += 4) {
    data_blue.data()[i + 2] = 255;
    data_blue.data()[i + 3] = 255;
  }
  texture::CreateTexture2DParam param_blue{1, lod2_size, lod2_size, {data_red.data()}};
  context->CreateTexture2D("blue", param_blue);
  lod_texture2d_level_2_ = context->GetTexture("blue");

  context->CreateTexture2D(
      "texture2d_lod_rgb", {3, lod0_size, lod0_size, {data_red.data(), data_green.data(), data_blue.data()}});
  lod_texture2d_ = context->GetTexture("texture2d_lod_rgb");
}

void TextureLodScene::InitCubemapLod(Context *context) {
  int level0_size = 512;
  std::array<std::vector<GLubyte*>, 6> datas;
  for (int texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset) {
    datas[texture_unit_offset].resize(3);

    // Red
    int lod0_size = level0_size;
    std::vector<GLubyte> data_red(lod0_size * lod0_size * 4 * 4);
    for (int i = 0; i < lod0_size * lod0_size * 4; i += 4) {
      data_red.data()[i] = 255;
      data_red.data()[i + 3] = 255;
    }
    datas[texture_unit_offset][0] = data_red.data();

    // Green
    int lod1_size = 256;
    std::vector<GLubyte> data_green(lod1_size * lod1_size * 4 * 4);
    for (int i = 0; i < lod1_size * lod1_size * 4; i += 4) {
      data_green.data()[i + 1] = 255;
      data_green.data()[i + 3] = 255;
    }
    datas[texture_unit_offset][1] = data_green.data();

    // Blue
    int lod2_size = 128;
    std::vector<GLubyte> data_blue(lod2_size * lod2_size * 4 * 4);
    for (int i = 0; i < lod2_size * lod2_size * 4; i += 4) {
      data_blue.data()[i + 2] = 255;
      data_blue.data()[i + 3] = 255;
    }
    datas[texture_unit_offset][2] = data_blue.data();
  }

  context->CreateCubemap("cubemap_lod_rgb", {3, level0_size, level0_size, datas});
  lod_cubemap_ = context->GetTexture("cubemap_lod_rgb"); 
}

void TextureLodScene::OnUpdate(Context *context) {
  OnUpdateCommon _(context, "TextureLodScene");
}

void TextureLodScene::OnRender(Context *context) {
  Cube cube1;
  cube1.mutable_transform()->SetTranslation(glm::vec3(-1.5, 0, 0));
  Texture2DLodShader({lod_texture2d_, context->camera().transform().translation()}, context, &cube1);
  cube1.OnRender(context);

  Cube cube2;
  cube1.mutable_transform()->SetTranslation(glm::vec3(1.5, 0, 0));
  CubemapLodShader({lod_cubemap_, context->camera().transform().translation()}, context, &cube2);
  cube2.OnRender(context);
}

void TextureLodScene::OnExit(Context *context) {
}