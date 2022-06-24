#include "renderer/config.h"

#include "renderer/debug.h"
#include "renderer/util.h"

namespace renderer {

void Config::Init(const std::string& config_path) {
  ConfigData config_data;
  std::string content;
  util::ReadFileToString(config_path, &content);
  util::ParseFromString(content, &config_data);
  for (const ShaderConfig& shader_config : config_data.shader_config()) {
    shader_configs_[shader_config.name()] = shader_config;
  }
  for (const TextureConfig& texture_config: config_data.texture_config()) {
    texture_configs_[texture_config.name()] = texture_config;
  }
  for (const ModelConfig& model_config : config_data.model_config()) {
    model_configs_[model_config.name()] = model_config;
  }
}

const ShaderConfig& Config::shader_config(const std::string& name) const {
  CGCHECK(shader_configs_.find(name) != shader_configs_.end()) << name;
  return shader_configs_.at(name);
}

const TextureConfig& Config::texture_config(const std::string& name) const {
  CGCHECK(texture_configs_.find(name) != texture_configs_.end()) << name;
  return texture_configs_.at(name);
}

const ModelConfig& Config::model_config(const std::string& name) const {
  CGCHECK(model_configs_.find(name) != model_configs_.end()) << name;
  return model_configs_.at(name);
}

} // namespace renderer