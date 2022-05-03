#include "playground/pass.h"

void DepthBufferPass::Init(const engine::DepthFramebuffer::Option& depth_framebuffer_option,
                           const engine::Transform& camera_transform) {
  camera_->SetType(engine::Camera::Orthographic);
  camera_->SetTransform(camera_transform);
  depth_framebuffer_.Init(depth_framebuffer_option);
}

void DepthBufferPass::Begin() {
  depth_framebuffer_.Bind();
}

void DepthBufferPass::End() {
  depth_framebuffer_.Unbind();
}

void ForwardPass::Init(const engine::ColorFramebuffer::Option& option) {
  color_framebuffer_.Init(option);
}

void ForwardPass::Begin() {
  color_framebuffer_.Bind();
}

void ForwardPass::End() {
  color_framebuffer_.Unbind();
}

void ShadowPass::Init(const glm::mat4& camera_vp, const engine::Texture& GetDepthTexture,
                      const engine::Texture& GetColorTexture) {
  camera_vp_ = camera_vp;
  depth_texture_ = GetDepthTexture;
  color_texture_ = GetColorTexture;
}

void ShadowPass::Begin() {
}

void ShadowPass::End() {
}