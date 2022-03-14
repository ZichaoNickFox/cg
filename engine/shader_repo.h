#pragma once

#include <unordered_map>

#include "engine/config.pb.h"
#include "engine/shader.h"

namespace engine {
class ShaderRepo {
 public:
  void Init(const Config& config);
  std::shared_ptr<Shader> GetOrLoadShader(const std::string& name);

 private:
  struct ShaderData {
    ShaderData() {}
    ShaderData(const ShaderConfig& in_config) { config = in_config; }
    bool loaded = false;
    std::shared_ptr<Shader> shader;
    ShaderConfig config;
  };
  std::unordered_map<std::string, ShaderData> shaders_;
};
}