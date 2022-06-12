#pragma once

#include <unordered_map>

#include "engine/proto/config.pb.h"
#include "engine/shader.h"

namespace engine {
class ShaderRepo {
 public:
  void Init(const Config& config);
  engine::Shader GetOrLoadShader(const std::string& name);
  void ReloadShaders();

 private:
  struct ShaderLoadState {
    ShaderLoadState(const ShaderConfig& in_config) { config = in_config; }
    bool loaded = false;
    engine::Shader shader;
    ShaderConfig config;
  };
  std::unordered_map<std::string, ShaderLoadState> shaders_;
};
} // namespace engine