#pragma once

#include "engine/camera.h"
#include "engine/frame_buffer/depth_frame_buffer.h"
#include "playground/context.h"
#include "playground/object/object.h"

class DirectionalLight : public Object {
 public:
  void Init();
  void ShadowMapRenderBegin(Context* context);
  void ShadowMapRenderEnd(Context* context);
  engine::Texture GetShadowMap() { return depth_frame_buffer_.texture(); }

 private:
  std::shared_ptr<engine::Camera> shadow_map_camera_ = std::make_shared<engine::Camera>();

  static constexpr int shadow_map_width = 1024;
  static constexpr int shadow_map_height = 1024;
  engine::DepthFrameBuffer depth_frame_buffer_;
};