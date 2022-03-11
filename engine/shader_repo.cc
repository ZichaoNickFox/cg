#include "engine/shader_repo.h"

#include <glog/logging.h>

#include "engine/file_util.h"
#include "engine/proto_util.h"

namespace engine {
void ShaderRepo::Init(const Config& config) {
  std::string content;
  for (const ShaderConfig& shader_config : config.shader_config()) {
    std::string name = shader_config.name();
    std::string vs;
    std::string fs;
    file_util::ReadFileToString(shader_config.vs_path(), &vs); 
    file_util::ReadFileToString(shader_config.fs_path(), &fs); 
    LOG(ERROR) << "Compiling shader " << name;
    shaders_.insert(std::make_pair(name, Shader(vs, fs)));
  }
}

Shader ShaderRepo::GetShader(const std::string& name) const {
  CHECK(shaders_.count(name) > 0) << "No shader name : " << name;
  return shaders_.at(name);
}
}