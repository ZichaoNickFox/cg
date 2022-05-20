#pragma once

#include <unordered_map>

#include "engine/camera.h"
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
  void Init(engine::Framebuffer* depth_framebuffer, const engine::Transform& camera_transform);
  void Begin() override;
  void End() override;

  std::shared_ptr<engine::Camera> mutable_camera() const { return camera_; }
  std::shared_ptr<const engine::Camera> camera() const { return camera_; }
  glm::mat4 camera_vp() { return camera_->GetProjectMatrix() * camera_->GetViewMatrix(); }
  engine::Texture GetTexture(const std::string& name) { return depth_framebuffer_->GetTexture(name); }
  ShaderShadowInfo shader_shadow_info() { return ShaderShadowInfo{camera_vp(), GetTexture(engine::kAttachmentNameDepth)}; }
  
 private:
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  engine::Framebuffer* depth_framebuffer_;
};

class ForwardPass : public Pass {
 public:
  void Init(engine::Framebuffer* forward_framebuffer);
  void Update(const ShaderShadowInfo& prepass_shadow_info) { prepass_shadow_info_ = prepass_shadow_info; }
  void Begin() override;
  void End() override;

  const ShaderShadowInfo& prepass_shadow_info() { return prepass_shadow_info_; }
  engine::Texture GetTexture(const std::string& name) { return forward_framebuffer_->GetTexture(name); }

 private:
  ShaderShadowInfo prepass_shadow_info_;
  engine::Framebuffer* forward_framebuffer_;
};

class ShadowPass : public Pass {
 public:
  void Init(const glm::mat4& camera_vp, const engine::Texture& depth_texture,
            const engine::Texture& color_texture);
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