#include "playground/context.h"

void Context::Init(const Config& config) {
  shader_repo_.Init(config);
  texture_repo_.Init(config);
}

const engine::Camera& Context::camera() {
  BTCHECK(!cameras_.empty());
  BTCHECK(!cameras_.top().expired());
  return *cameras_.top().lock().get();
}

engine::Camera* Context::mutable_camera() {
  BTCHECK(!cameras_.empty());
  BTCHECK(!cameras_.top().expired());
  return cameras_.top().lock().get();
}

void Context::SetFrameInternal(int frame_interval) {
  frame_interval_ = frame_interval;
  fps_ = 1e3 / frame_interval_;
}