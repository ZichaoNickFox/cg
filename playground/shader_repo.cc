#include "playground/shader_repo.h"

#include <glog/logging.h>

#include "playground/util.h"

using engine::Shader;

void ShaderRepo::Init(const Config& config) {
  std::string content;
  for (const ShaderConfig& shader_config : config.shader_config()) {
    std::string name = shader_config.name();
    shaders_.insert(std::make_pair(name, ShaderData(shader_config)));
    LOG(ERROR) << "Init shader : " << name;
  }
}

Shader ShaderRepo::GetOrLoadShader(const std::string& name) {
  CHECK(shaders_.count(name) > 0) << "No shader name : " << name;
  ShaderData* shader_data = &shaders_[name];
  if (shader_data->loaded == false) {
    std::string vs;
    std::string fs;
    std::string gs;
    std::string ts;
    bool has_gs = shader_data->config.has_gs_path();
    bool has_ts = shader_data->config.has_ts_path();
    util::ReadFileToString(shader_data->config.vs_path(), &vs); 
    util::ReadFileToString(shader_data->config.fs_path(), &fs); 
    if (has_gs) {
      util::ReadFileToString(shader_data->config.gs_path(), &gs); 
    }
    if (has_ts) {
      util::ReadFileToString(shader_data->config.ts_path(), &ts); 
    }
    CHECK(vs.size() > 0) << "Load vs failed : " << name;
    CHECK(fs.size() > 0) << "Load fs failed : " << name;
    if (has_gs) {
      CHECK(gs.size() > 0) << "Load gs failed : " << name;
    }
    if (has_ts) {
      CHECK(ts.size() > 0) << "Load ts failed : " << name;
    }
    LOG(ERROR) << "Compiling shader " << name;
    shader_data->loaded = true;
    shader_data->shader = Shader(name, vs, fs, gs, ts);
  }
  return shader_data->shader;
}