#include "engine/repo/shader_repo.h"

#include "engine/debug.h"
#include "engine/shader_loader.h"
#include "engine/util.h"

namespace engine {
void ShaderRepo::Init(const Config& config) {
  std::string content;
  for (const ShaderConfig& shader_config : config.shader_config()) {
    std::string name = shader_config.name();
    shaders_.insert(std::make_pair(name, ShaderLoadState(shader_config)));
    CGLOG(ERROR) << "Init shader : " << name;
  }
}

Shader ShaderRepo::GetOrLoadShader(const std::string& name) {
  CGCHECK(shaders_.count(name) > 0) << "No shader name : " << name;
  ShaderLoadState* shader_load_state = &shaders_.at(name);
  if (shader_load_state->loaded == false) {
    ShaderLoader shader_loader;
    shader_load_state->shader = shader_loader.Load(name, {
        {ShaderLoader::kVS, shader_load_state->config.has_vs_path() ? shader_load_state->config.vs_path() : ""},
        {ShaderLoader::kFS, shader_load_state->config.has_fs_path() ? shader_load_state->config.fs_path() : ""},
        {ShaderLoader::kGS, shader_load_state->config.has_gs_path() ? shader_load_state->config.gs_path() : ""},
        {ShaderLoader::kTS, shader_load_state->config.has_ts_path() ? shader_load_state->config.ts_path() : ""},
        {ShaderLoader::kCS, shader_load_state->config.has_cs_path() ? shader_load_state->config.cs_path() : ""}});
    shader_load_state->loaded = true;
  }
  return shader_load_state->shader;
}

void ShaderRepo::ReloadShaders() {
  for (auto& p : shaders_) {
    p.second.loaded = false;
  }
}
} // namespace engine