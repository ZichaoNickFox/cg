#include "playground/pass.h"

void DepthBufferPass::Init(const engine::DepthFrameBuffer::Option& depth_frame_buffer_option,
                           const engine::Transform& camera_transform) {
  camera_->SetType(engine::Camera::Orthographic);
  camera_->SetTransform(camera_transform);
  depth_frame_buffer_.Init(depth_frame_buffer_option);
}

void DepthBufferPass::Begin() {
  depth_frame_buffer_.Bind();
}

void DepthBufferPass::End() {
  depth_frame_buffer_.Unbind();
}

void ForwardPass::Init(const engine::ColorFrameBuffer::Option& option) {
  color_frame_buffer_.Init(option);
}

void ForwardPass::Begin() {
  color_frame_buffer_.Bind();
}

void ForwardPass::End() {
  color_frame_buffer_.Unbind();
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