#pragma once

#include <unordered_map>

#include "renderer/camera.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/shadow.h"
#include "renderer/texture.h"

namespace renderer {
  
/*
class Pass {
 public:
  virtual void Begin() = 0;
  virtual void End() = 0;
};

class DepthBufferPass : public Pass {
 public:
  void Init(Framebuffer* depth_framebuffer, std::shared_ptr<Camera> camera);
  void Begin() override;
  void End() override;

  glm::mat4 camera_vp() { return camera_->GetProjectMatrix() * camera_->GetViewMatrix(); }
  Texture GetTexture(const std::string& name) { return depth_framebuffer_->GetTexture(name); }
  Shadow shadow();
  std::shared_ptr<Camera> camera() { return camera_; }
  
 private:
  std::shared_ptr<Camera> camera_;
  Framebuffer* depth_framebuffer_;
};

class ForwardPass : public Pass {
 public:
  void Init(Framebuffer* forward_framebuffer);
  void Update(const Shadows& shadows) { shadows_ = shadows; }
  void Begin() override;
  void End() override;

  const Shadows& shadows() { return shadows_; }
  Texture GetTexture(const std::string& name) { return forward_framebuffer_->GetTexture(name); }

 private:
  Shadows shadows_;
  Framebuffer* forward_framebuffer_;
};

class ShadowPass : public Pass {
 public:
  void Init(const glm::mat4& camera_vp, const Texture& depth_texture,
            const Texture& color_texture);
  void Begin() override;
  void End() override;

  glm::mat4 camera_vp() { return camera_vp_; }
  Texture GetDepthTexture() { return depth_texture_; }
  Texture GetColorTexture() { return color_texture_; }

 private:
  glm::mat4 camera_vp_;
  Texture depth_texture_;
  Texture color_texture_;
};

class GBufferPass : public Pass {
 public:
  void Init(Framebuffer* g_buffer) { g_buffer_ = g_buffer; }
  void Begin() override { g_buffer_->Bind(); }
  void End() override { g_buffer_->Unbind(); }
  Framebuffer* g_buffer() { return g_buffer_; }
 private:
  Framebuffer* g_buffer_;
};

class SSAOPass : public Pass {
 public:
  void Init(Framebuffer* SSAO_buffer);
  void Begin() override { SSAO_buffer_->Bind(); }
  void End() override { SSAO_buffer_->Unbind(); }

  renderer::Texture texture_position_vs;
  renderer::Texture texture_normal_vs;
  renderer::Texture texture_noise;
  renderer::Texture texture_position_ws;
  renderer::Texture texture_normal_ws;
  std::array<glm::vec3, 64> samples_ts;
  renderer::Texture texture_SSAO() { return SSAO_buffer_->GetTexture(kAttachmentColor.name); }
  
  Framebuffer* SSAO_buffer_;
};

class BlurPass : public Pass {
 public:
  void Init(Framebuffer* blur_buffer);
  void Begin() override { blur_buffer_->Bind(); }
  void End() override { blur_buffer_->Unbind(); }

  renderer::Texture texture_SSAO;
  renderer::Texture texture_blur() { return blur_buffer_->GetTexture(kAttachmentColor.name); }

 private:
  Framebuffer* blur_buffer_;
};

class LightingPass : public Pass {
 public:
  void Init(Framebuffer* in, Framebuffer* out);
  void Begin() override { in_->Bind(); }
  void End() override { in_->Unbind(); }
 private:
  Framebuffer* in_;
  Framebuffer* out_;
};
*/
} // namespace renderer