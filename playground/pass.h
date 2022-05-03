#pragma once

#include <unordered_map>

#include "engine/camera.h"
#include "engine/framebuffer/color_framebuffer.h"
#include "engine/framebuffer/depth_framebuffer.h"
#include "engine/gl.h"
#include "engine/texture.h"
#include "playground/shaders.h"

class Pass {
 public:
  virtual void Begin() = 0;
  virtual void End() = 0;
};

class DepthBufferPass : public Pass {
 public:
  void Init(const engine::DepthFramebuffer::Option& depth_framebuffer_option,
            const engine::Transform& camera_transform);
  void Begin() override;
  void End() override;

  std::shared_ptr<engine::Camera> mutable_camera() const { return camera_; }
  std::shared_ptr<const engine::Camera> camera() const { return camera_; }
  glm::mat4 camera_vp() { return camera_->GetProjectMatrix() * camera_->GetViewMatrix(); }
  engine::Texture GetDepthTexture() { return depth_framebuffer_.GetDepthTexture(); }
  ShaderShadowInfo shader_shadow_info() { return ShaderShadowInfo{camera_vp(), GetDepthTexture()}; }
  
 private:
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  engine::DepthFramebuffer depth_framebuffer_;
};

class ForwardPass : public Pass {
 public:
  void Init(const engine::ColorFramebuffer::Option& option);
  void Update(const ShaderShadowInfo& prepass_shadow_info) { prepass_shadow_info_ = prepass_shadow_info; }
  void Begin() override;
  void End() override;

  const ShaderShadowInfo& prepass_shadow_info() { return prepass_shadow_info_; }
  engine::Texture GetColorTexture(int i = 0) { return color_framebuffer_.GetColorTexture(i); }
  engine::Texture GetDepthTexture() { return color_framebuffer_.GetDepthTexture(); }

 private:
  ShaderShadowInfo prepass_shadow_info_;
  engine::ColorFramebuffer color_framebuffer_;
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