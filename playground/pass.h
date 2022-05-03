#pragma once

#include "GL/glew.h"
#include <unordered_map>

#include "engine/camera.h"
#include "engine/frame_buffer/color_frame_buffer.h"
#include "engine/frame_buffer/depth_frame_buffer.h"
#include "engine/texture.h"
#include "playground/shaders.h"

class Pass {
 public:
  virtual void Begin() = 0;
  virtual void End() = 0;
};

class DepthBufferPass : public Pass {
 public:
  void Init(const engine::DepthFrameBuffer::Option& depth_frame_buffer_option,
            const engine::Transform& camera_transform);
  void Begin() override;
  void End() override;

  std::shared_ptr<engine::Camera> mutable_camera() const { return camera_; }
  std::shared_ptr<const engine::Camera> camera() const { return camera_; }
  glm::mat4 camera_vp() { return camera_->GetProjectMatrix() * camera_->GetViewMatrix(); }
  engine::Texture GetDepthTexture() { return depth_frame_buffer_.GetDepthTexture(); }
  ShaderShadowInfo shader_shadow_info() { return ShaderShadowInfo{camera_vp(), GetDepthTexture()}; }
  
 private:
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  engine::DepthFrameBuffer depth_frame_buffer_;
};

class ForwardPass : public Pass {
 public:
  void Init(const engine::ColorFrameBuffer::Option& option);
  void Update(const ShaderShadowInfo& prepass_shadow_info) { prepass_shadow_info_ = prepass_shadow_info; }
  void Begin() override;
  void End() override;

  const ShaderShadowInfo& prepass_shadow_info() { return prepass_shadow_info_; }
  engine::Texture GetColorTexture(int i = 0) { return color_frame_buffer_.GetColorTexture(i); }
  engine::Texture GetDepthTexture() { return color_frame_buffer_.GetDepthTexture(); }

 private:
  ShaderShadowInfo prepass_shadow_info_;
  engine::ColorFrameBuffer color_frame_buffer_;
};

class ShadowPass : public Pass {
 public:
  void Init(const glm::mat4& camera_vp, const engine::Texture& depth_texture,
            const engine::Texture& GetColorTexture);
  void Begin() override;
  void End() override;

  glm::mat4 camera_vp() { return camera_vp_; }
  engine::Texture GetDepthTexture() { return depth_texture_; }
  engine::Texture GetColorTexture() { return color_texture_; }

 private:
  glm::mat4 camera_vp_;
  engine::Texture depth_texture_;
  engine::Texture color_texture_;
};