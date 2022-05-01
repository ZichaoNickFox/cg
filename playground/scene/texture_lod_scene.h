#pragma once

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>

#include "engine/camera.h"
#include "engine/color_frame_buffer.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/empty_object.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class TextureLodScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  void InitTexture2DLod(Context *context);
  void InitCubemapLod(Context *context);

  engine::Texture lod_texture2d_level_0_;
  engine::Texture lod_texture2d_level_1_;
  engine::Texture lod_texture2d_level_2_;
  engine::Texture lod_texture2d_;

  engine::Texture lod_cubemap_level_0_;
  engine::Texture lod_cubemap_level_1_;
  engine::Texture lod_cubemap_level_2_;
  engine::Texture lod_cubemap_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  engine::ColorFrameBuffer color_frame_buffer_;
};