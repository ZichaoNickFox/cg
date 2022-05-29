#pragma once

#include <unordered_map>

#include "engine/camera.h"
#include "engine/framebuffer.h"
#include "engine/gl.h"
#include "engine/texture.h"

namespace engine {
struct SceneShadowInfo {
  struct ShadowInfo {
    glm::mat4 light_space_vp;
    Texture texture_depth;
  };
  SceneShadowInfo() {}
  SceneShadowInfo(const ShadowInfo& in_info) : infos(in_info) {}
  ShadowInfo infos;
};

struct SceneLightInfo {
  struct LightInfo {
    enum Type {
      kDirectionalLight = 0,
      kPointLight,
      kSpotLight
    };
    Type type;
    glm::vec3 pos;
    glm::vec3 color;
    uint32_t attenuation_metre;
  };
  SceneLightInfo() {}
  void Insert(const LightInfo& in_info);
  void Insert(const std::vector<LightInfo>& in_infos);
  std::vector<LightInfo> infos;
};
  
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
  SceneShadowInfo scene_shadow_info();
  std::shared_ptr<Camera> camera() { return camera_; }
  
 private:
  std::shared_ptr<Camera> camera_;
  Framebuffer* depth_framebuffer_;
};

class ForwardPass : public Pass {
 public:
  void Init(Framebuffer* forward_framebuffer);
  void Update(const SceneShadowInfo& prepass_shadow_info) { prepass_shadow_info_ = prepass_shadow_info; }
  void Begin() override;
  void End() override;

  const SceneShadowInfo& prepass_shadow_info() { return prepass_shadow_info_; }
  Texture GetTexture(const std::string& name) { return forward_framebuffer_->GetTexture(name); }

 private:
  SceneShadowInfo prepass_shadow_info_;
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

  engine::Texture texture_position_vs;
  engine::Texture texture_normal_vs;
  engine::Texture texture_noise;
  engine::Texture texture_position_ws;
  engine::Texture texture_normal_ws;
  std::array<glm::vec3, 64> samples_ts;
  engine::Texture texture_SSAO() { return SSAO_buffer_->GetTexture(kAttachmentColor.name); }
  
  Framebuffer* SSAO_buffer_;
};

class BlurPass : public Pass {
 public:
  void Init(Framebuffer* blur_buffer);
  void Begin() override { blur_buffer_->Bind(); }
  void End() override { blur_buffer_->Unbind(); }

  engine::Texture texture_SSAO;
  engine::Texture texture_blur() { return blur_buffer_->GetTexture(kAttachmentColor.name); }

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
} // namespace engine