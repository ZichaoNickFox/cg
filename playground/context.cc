#include "playground/context.h"

#include "playground/util.h"

void Context::Init(const Option& option) {
  std::string content;
  util::ReadFileToString(option.config_path, &content);
  Config config;
  util::ParseFromString(content, &config);

  screen_width_ = option.screen_width;
  screen_height_ = option.screen_height;
  shader_repo_.Init(config);
  texture_repo_.Init(config);
  mesh_repo_.Init(config);
  model_repo_.Init(config);

  clear_color_ = option.clear_color;
}

const engine::Camera& Context::camera() {
  CGCHECK(!cameras_.empty());
  CGCHECK(!cameras_.top().expired());
  return *cameras_.top().lock().get();
}

engine::Camera* Context::mutable_camera() {
  CGCHECK(!cameras_.empty());
  CGCHECK(!cameras_.top().expired());
  return cameras_.top().lock().get();
}

void Context::SetFrameInternal(int frame_interval) {
  frame_interval_ = frame_interval;
  fps_ = 1e3 / frame_interval_;
}

engine::Shader Context::GetShader(const std::string& name) {
  return shader_repo_.GetOrLoadShader(name);
}

engine::Texture Context::GetTexture(const std::string& name) {
  return texture_repo_.GetOrLoadTexture(name);
}

std::shared_ptr<engine::Mesh> Context::GetMesh(const std::string& name) {
  return mesh_repo_.GetOrLoadMesh(name);
}