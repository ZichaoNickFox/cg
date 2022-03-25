#pragma once

#include <unordered_map>

#include "engine/shader.h"
#include "playground/config.pb.h"

class ShaderRepo {
 public:
  void Init(const Config& config);
  engine::Shader GetOrLoadShader(const std::string& name);

 private:
  struct ShaderData {
    ShaderData(const ShaderConfig& in_config) { config = in_config; }
    bool loaded = false;
    engine::Shader shader;
    ShaderConfig config;
  };
  std::unordered_map<std::string, ShaderData> shaders_;
};