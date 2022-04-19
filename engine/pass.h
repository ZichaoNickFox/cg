#pragma once

#include "GL/glew.h"
#include <unordered_map>

#include "engine/camera.h"
#include "engine/depth_frame_buffer.h"
#include "engine/texture.h"

namespace engine {
class Pass {
 public:
  virtual void Begin() = 0;
  virtual void End() = 0;
};

class ZBufferPass : public Pass {
 public:
  ZBufferPass(const DepthFrameBuffer::Option& depth_frame_buffer_option, const engine::Transform& camera_transform);
  void Begin() override;
  void End() override;

  const engine::Camera& camera() { return *camera_.get(); }
  glm::mat4 camera_vp() { return camera_->GetProjectMatrix() * camera_->GetViewMatrix(); }
  Texture z_buffer_texture() { return depth_frame_buffer_.GetTexture(); }
  
 private:
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  engine::DepthFrameBuffer depth_frame_buffer_;
};

class ForwardPass : public Pass {
 public:
  void Begin() override;
  void End() override;
};

class ShadowPass : public Pass {
 public:
  ShadowPass(const glm::mat4& camera_vp, const Texture& z_buffer_texture);
  void Begin() override;
  void End() override;

  glm::mat4 camera_vp() { return camera_vp_; }
  Texture z_buffer_texture() { return z_buffer_texture_; }

 private:
  glm::mat4 camera_vp_;
  Texture z_buffer_texture_;
};
} // namespace engine