#include "engine/shader_repo.h"

#include <glog/logging.h>

#include "engine/file_util.h"
#include "engine/proto_util.h"

namespace engine {
void ShaderRepo::Init(const Config& config) {
  std::string content;
  for (const ShaderConfig& shader_config : config.shader_config()) {
    std::string name = shader_config.name();
    shaders_.insert(std::make_pair(name, ShaderData(shader_config)));
  }
}

std::shared_ptr<Shader> ShaderRepo::GetOrLoadShader(const std::string& name) {
  CHECK(shaders_.count(name) > 0) << "No shader name : " << name;
  ShaderData* shader_data = &shaders_[name];
  if (shader_data->loaded == false) {
    std::string vs;
    std::string fs;
    file_util::ReadFileToString(shader_data->config.vs_path(), &vs); 
    file_util::ReadFileToString(shader_data->config.fs_path(), &fs); 
    CHECK(vs.size() > 0) << "Load vs failed : " << name;
    CHECK(fs.size() > 0) << "Load fs failed : " << name;
    LOG(ERROR) << "Compiling shader " << name;
    shader_data->loaded = true;
    shader_data->shader = std::make_shared<Shader>(name, vs, fs);
  }
  return shader_data->shader;
}
}