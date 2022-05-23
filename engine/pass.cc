#include "engine/pass.h"

namespace engine {
void SceneLightInfo::Insert(const LightInfo& in_light_info) {
  infos.push_back(in_light_info);
}

void SceneLightInfo::Insert(const std::vector<LightInfo>& in_light_infos) {
  infos.insert(infos.end(), in_light_infos.begin(), in_light_infos.end());
}

void DepthBufferPass::Init(Framebuffer* depth_framebuffer, std::shared_ptr<Camera> camera) {
  depth_framebuffer_ = depth_framebuffer;
  camera_ = camera;
}

void DepthBufferPass::Begin() {
  depth_framebuffer_->Bind();
}

void DepthBufferPass::End() {
  depth_framebuffer_->Unbind();
}

SceneShadowInfo DepthBufferPass::scene_shadow_info() {
  return SceneShadowInfo::ShadowInfo{camera_vp(), GetTexture(kAttachmentNameDepth)};
}

void ForwardPass::Init(Framebuffer* forward_framebuffer) {
  forward_framebuffer_ = forward_framebuffer;
}

void ForwardPass::Begin() {
  forward_framebuffer_->Bind();
}

void ForwardPass::End() {
  forward_framebuffer_->Unbind();
}

void ShadowPass::Init(const glm::mat4& camera_vp, const Texture& depth_texture,
                      const Texture& color_texture) {
  camera_vp_ = camera_vp;
  depth_texture_ = depth_texture;
  color_texture_ = color_texture;
}

void ShadowPass::Begin() {
}

void ShadowPass::End() {
}

void SSAOPass::Init(Framebuffer* SSAO_buffer) {
  SSAO_buffer_ = SSAO_buffer;
}

void BlurPass::Init(Framebuffer* blur_buffer) {
  blur_buffer_ = blur_buffer;
}

void LightingPass::Init(Framebuffer* in, Framebuffer* out) {
  in_ = in;
  out_ = out;
}
} // namespace engine