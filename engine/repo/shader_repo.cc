#include "engine/repo/shader_repo.h"

#include <glog/logging.h>

#include "engine/util.h"

namespace engine {
void ShaderRepo::Init(const Config& config) {
  std::string content;
  for (const ShaderConfig& shader_config : config.shader_config()) {
    std::string name = shader_config.name();
    shaders_.insert(std::make_pair(name, ShaderData(shader_config)));
    CGLOG(ERROR) << "Init shader : " << name;
  }
}

Shader ShaderRepo::GetOrLoadShader(const std::string& name) {
  CGCHECK(shaders_.count(name) > 0) << "No shader name : " << name;
  ShaderData* shader_data = &shaders_.at(name);
  if (shader_data->loaded == false) {
    std::vector<Shader::CodePart> vs(shader_data->config.vs_path_size());
    std::vector<Shader::CodePart> fs(shader_data->config.fs_path_size());
    std::vector<Shader::CodePart> gs(shader_data->config.gs_path_size());
    std::vector<Shader::CodePart> ts(shader_data->config.ts_path_size());
    for (int i = 0; i < vs.size(); ++i) {
      const std::string path = shader_data->config.vs_path(i);
      vs[i].glsl_path = path;
      util::ReadFileToString(path, &vs[i].code); 
      CGCHECK(vs[i].code.size() > 0) << " vs path error : " << path;
    }
    for (int i = 0; i < fs.size(); ++i) {
      const std::string path = shader_data->config.fs_path(i);
      fs[i].glsl_path = path;
      util::ReadFileToString(path, &fs[i].code); 
      CGCHECK(fs[i].code.size() > 0) << " fs path error : " << path;
    }
    for (int i = 0; i < gs.size(); ++i) {
      const std::string path = shader_data->config.gs_path(i);
      gs[i].glsl_path = path;
      util::ReadFileToString(path, &gs[i].code); 
      CGCHECK(gs[i].code.size() > 0) << " gs path error : " << path;
    }
    for (int i = 0; i < ts.size(); ++i) {
      const std::string path = shader_data->config.ts_path(i);
      ts[i].glsl_path = path;
      util::ReadFileToString(path, &ts[i].code); 
      CGCHECK(ts[i].code.size() > 0) << " ts path error : " << path;
    }
    CGLOG(ERROR) << "Compiling shader " << name;
    shader_data->shader = Shader(name, vs, fs, gs, ts);
    shader_data->loaded = true;
  }
  return shader_data->shader;
}
} // namespace engine