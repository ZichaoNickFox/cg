#include "playground/context.h"

#include "engine/debug.h"
#include "engine/util.h"

void Context::Init(const Option& option) {
  std::string content;
  util::ReadFileToString(option.config_path, &content);
  Config config;
  util::ParseFromString(content, &config);

  shader_repo_.Init(config);
  texture_repo_.Init(config);
  mesh_repo_.Init(config);
  model_repo_.Init(config);

  clear_color_ = option.clear_color;
  framebuffer_size_ = option.framebuffer_size;
  set_clipboard_string_func_ = option.set_clipboard_string_func;
}

const engine::Camera& Context::camera() const {
  CGCHECK(camera_) << "camera must be nullptr";
  return *camera_;
}

engine::Camera* Context::mutable_camera() {
  return camera_;
}

void Context::StatFrame(int last_frame_interval) {
  frame_stat_.OnFrame(last_frame_interval);
}

engine::Shader Context::GetShader(const std::string& name) {
  return shader_repo_.GetOrLoadShader(name);
}

engine::Texture Context::GetTexture(const std::string& name, bool flip_vertically, bool equirectangular) {
  return texture_repo_.GetOrLoadTexture(name, flip_vertically);
}

void Context::SaveTexture2D(const std::string& name) {
  texture_repo_.SaveTexture2D(name);
}

void Context::SaveCubemap(const std::string& name) {
  texture_repo_.SaveCubemap(name);
}

void Context::ResetTexture2D(const std::string& name, const engine::TextureParam& param) {
  texture_repo_.ResetTexture2D(name, param);
}

void Context::ResetCubemap(const std::string& name, const engine::CubemapParam& param) {
  texture_repo_.ResetCubemap(name, param);
}

engine::Texture Context::CreateTexture(const engine::TextureParam& param) {
  return texture_repo_.CreateTexture(param);
}

engine::Texture Context::CreateCubemapPreviewTexture2D(const engine::CubemapParam& param) {
  return texture_repo_.CreateCubemapPreviewTexture2D(param);
}

std::shared_ptr<const engine::Mesh> Context::GetMesh(const std::string& name) {
  return mesh_repo_.GetOrLoadMesh(name);
}

std::vector<engine::ModelRepo::ModelPartData> Context::GetModel(const std::string& name) {
  return model_repo_.GetOrLoadModel(name);
}
