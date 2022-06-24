#include "renderer/pass.h"

namespace renderer {

/*
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

Shadow DepthBufferPass::shadow() {
  return Shadow{camera_vp(), GetTexture(kAttachmentDepth.name)};
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
*/
} // namespace renderer