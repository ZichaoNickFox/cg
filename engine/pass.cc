#include "engine/pass.h"

namespace engine {
ZBufferPass::ZBufferPass(const DepthFrameBuffer::Option& depth_frame_buffer_option,
                         const engine::Transform& camera_transform) {
  camera_->SetType(engine::Camera::Perspective);
  camera_->SetTransform(camera_transform);
  depth_frame_buffer_.Init(depth_frame_buffer_option);
}

void ZBufferPass::Begin() {
  depth_frame_buffer_.Bind();
}

void ZBufferPass::End() {
  depth_frame_buffer_.Unbind();
}

void ForwardPass::Begin() {
}

void ForwardPass::End() {
}

ShadowPass::ShadowPass(const glm::mat4& camera_vp, const Texture& z_buffer_texture) {
  camera_vp_ = camera_vp;
  z_buffer_texture_ = z_buffer_texture;
}

void ShadowPass::Begin() {
}

void ShadowPass::End() {
}
} // namespace engine