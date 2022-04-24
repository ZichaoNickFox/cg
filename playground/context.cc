#include "playground/context.h"

#include "engine/debug.h"
#include "playground/util.h"

void Context::Init(const Option& option) {
  std::string content;
  util::ReadFileToString(option.config_path, &content);
  Config config;
  util::ParseFromString(content, &config);

  shader_repo_.Init(config);
  texture_repo_.Init(config);
  mesh_repo_.Init(config);
  model_repo_.Init(config);
  light_attenuation_config_ = config.light_attenuation_config();
  material_property_config_ = config.material_property_config();

  clear_color_ = option.clear_color;
  frame_buffer_size_ = option.frame_buffer_size;
}

const engine::Camera& Context::camera() {
  CGCHECK(!camera_.expired());
  return *camera_.lock().get();
}

engine::Camera* Context::mutable_camera() {
  CGCHECK(!camera_.expired());
  return camera_.lock().get();
}

void Context::SetFrameInternal(int frame_interval) {
  frame_interval_ = frame_interval;
  fps_ = 1e3 / frame_interval_;
}

engine::Shader Context::GetShader(const std::string& name) {
  return shader_repo_.GetOrLoadShader(name);
}

engine::Texture Context::GetTexture(const std::string& name, bool flip_vertically, bool use_mipmap,
                                    bool equirectangular) {
  return texture_repo_.GetOrLoadTexture(name, flip_vertically, use_mipmap);
}

void Context::SaveTexture(const std::string& name, const engine::Texture& texture) {
  texture_repo_.SaveTexture(name, texture);
}

std::shared_ptr<const engine::Mesh> Context::GetMesh(const std::string& name) {
  return mesh_repo_.GetOrLoadMesh(name);
}

std::vector<ModelRepo::ModelPartData> Context::GetModel(const std::string& name) {
  return model_repo_.GetOrLoadModel(name);
}

float Context::light_attenuation_constant(int metre) {
  CGCHECK(light_attenuation_config_.count(metre) > 0) << " : " << metre;
  return light_attenuation_config_[metre].constant();
}

float Context::light_attenuation_linear(int metre) {
  CGCHECK(light_attenuation_config_.count(metre) > 0) << " : " << metre;
  return light_attenuation_config_[metre].linear();
}

float Context::light_attenuation_quadratic(int metre) {
  CGCHECK(light_attenuation_config_.count(metre) > 0) << " : " << metre;
  return light_attenuation_config_[metre].quadratic();
}

glm::vec4 Context::material_property_ambient(const std::string& name) {
  CGCHECK(material_property_config_.count(name) > 0) << " : " << name;
  return glm::vec4(material_property_config_[name].ambient(0),
                   material_property_config_[name].ambient(1),
                   material_property_config_[name].ambient(2),
                   material_property_config_[name].ambient(3));
}

glm::vec4 Context::material_property_diffuse(const std::string& name) {
  CGCHECK(material_property_config_.count(name) > 0) << " : " << name;
  return glm::vec4(material_property_config_[name].diffuse(0),
                   material_property_config_[name].diffuse(1),
                   material_property_config_[name].diffuse(2),
                   material_property_config_[name].diffuse(3));
}

glm::vec4 Context::material_property_specular(const std::string& name) {
  CGCHECK(material_property_config_.count(name) > 0) << " : " << name;
  return glm::vec4(material_property_config_[name].specular(0),
                   material_property_config_[name].specular(1),
                   material_property_config_[name].specular(2),
                   material_property_config_[name].specular(3));
}

float Context::material_property_shininess(const std::string& name) {
  CGCHECK(material_property_config_.count(name) > 0) << " : " << name;
  return material_property_config_[name].shininess();
}
