#pragma once

#include <unordered_map>

#include "renderer/proto/config.pb.h"

namespace cg {

class Config {
 public:
  void Init(const std::string& config_path);
  const ShaderConfig& shader_config(const std::string& name) const;
  const TextureConfig& texture_config(const std::string& name) const;
  const ModelConfig& model_config(const std::string& name) const;
  const std::unordered_map<std::string, ShaderConfig>& shader_configs() const { return shader_configs_; }
  const std::unordered_map<std::string, TextureConfig>& texture_configs() const { return texture_configs_; }
  const std::unordered_map<std::string, ModelConfig>& model_configs() const { return model_configs_; }

 private:
  std::unordered_map<std::string, ShaderConfig> shader_configs_;
  std::unordered_map<std::string, TextureConfig> texture_configs_;
  std::unordered_map<std::string, ModelConfig> model_configs_;
};

} // namespace cg