#include "playground/pass.h"

void DepthBufferPass::Init(engine::Framebuffer* depth_framebuffer,
                           const engine::Transform& camera_transform) {
  camera_->SetType(engine::Camera::Orthographic);
  camera_->SetTransform(camera_transform);
  depth_framebuffer_ = depth_framebuffer;
}

void DepthBufferPass::Begin() {
  depth_framebuffer_->Bind();
}

void DepthBufferPass::End() {
  depth_framebuffer_->Unbind();
}

void ForwardPass::Init(engine::Framebuffer* forward_framebuffer) {
  forward_framebuffer_ = forward_framebuffer;
}

void ForwardPass::Begin() {
  forward_framebuffer_->Bind();
}

void ForwardPass::End() {
  forward_framebuffer_->Unbind();
}

void ShadowPass::Init(const glm::mat4& camera_vp, const engine::Texture& depth_texture,
                      const engine::Texture& color_texture) {
  camera_vp_ = camera_vp;
  depth_texture_ = depth_texture;
  color_texture_ = color_texture;
}

void ShadowPass::Begin() {
}

void ShadowPass::End() {
}