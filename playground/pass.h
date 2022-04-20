#pragma once

#include "GL/glew.h"
#include <unordered_map>

#include "engine/camera.h"
#include "engine/color_frame_buffer.h"
#include "engine/depth_frame_buffer.h"
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

  const engine::Camera& camera() { return *camera_.get(); }
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
  void Update(const ShaderShadowInfo& shader_shadow_info) { shader_shadow_info_ = shader_shadow_info; }
  void Begin() override;
  void End() override;
  engine::Texture GetDepthTexture() { return color_frame_buffer_.GetDepthTexture(); }
  engine::Texture GetColorTexture(int i = 0) { return color_frame_buffer_.GetColorTexture(i); }
  const ShaderShadowInfo& shader_shadow_info() { return shader_shadow_info_; }

 private:
  ShaderShadowInfo shader_shadow_info_;
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