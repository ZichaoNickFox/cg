#pragma once

#include <unordered_map>

#include "engine/config.pb.h"
#include "engine/shader.h"

namespace engine {
class ShaderRepo {
 public:
  void Init(const Config& config);
  std::shared_ptr<Shader> GetShader(const std::string& name) const;

 private:
  std::unordered_map<std::string, std::shared_ptr<Shader>> shaders_;
};
}