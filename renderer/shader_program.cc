#include "renderer/shader_program.h"

#include "glog/logging.h"

#include "base/debug.h"
#include "renderer/shader_loader.h"

namespace cg {

ShaderProgram::ShaderProgram(const std::string& name,
                             const std::vector<CodePart>& vs,
                             const std::vector<CodePart>& fs,
                             const std::vector<CodePart>& gs,
                             const std::vector<CodePart>& ts)
    : program_(rhi::GetDevice().CreateRenderProgram(name, vs, fs, gs, ts)),
      name_(name) {}

ShaderProgram::ShaderProgram(const std::string& name, const std::vector<CodePart>& cs)
    : program_(rhi::GetDevice().CreateComputeProgram(name, cs)),
      name_(name) {}

const rhi::Program& ShaderProgram::ProgramRef() const {
  return *CGCHECK_NOTNULL(program_.get());
}

void ShaderProgram::Use() const {
  ProgramRef().Use();
}

void ShaderProgram::SetBool(const std::string& location_name, bool value) const {
  ProgramRef().SetBool(location_name, value);
}

void ShaderProgram::SetInt(const std::string& location_name, int value) const {
  ProgramRef().SetInt(location_name, value);
}

void ShaderProgram::SetFloat(const std::string& location_name, float value) const {
  ProgramRef().SetFloat(location_name, value);
}

int ShaderProgram::SetTexture(const std::string& location_name, const Texture& value) const {
  return ProgramRef().BindTexture(location_name, value);
}

void ShaderProgram::SetMat4(const std::string& location_name, const glm::mat4& value) const {
  ProgramRef().SetMat4(location_name, value);
}

void ShaderProgram::SetVec4(const std::string& location_name, const glm::vec4& value) const {
  ProgramRef().SetVec4(location_name, value);
}

void ShaderProgram::SetVec3(const std::string& location_name, const glm::vec3& value) const {
  ProgramRef().SetVec3(location_name, value);
}

void ShaderProgram::SetVec2(const std::string& location_name, const glm::vec2& value) const {
  ProgramRef().SetVec2(location_name, value);
}

uint32_t ShaderProgram::id() const {
  return program_ == nullptr ? 0 : program_->id();
}

void ShaderProgramRepo::Init(const Config& config) {
  for (const auto& p : config.shader_configs()) {
    const ShaderConfig& shader_config = p.second;
    std::string name = shader_config.name();
    shaders_.insert(std::make_pair(name, ShaderLoadState(shader_config)));
    CGLOG(ERROR) << "Init shader : " << name;
  }
}

ShaderProgram ShaderProgramRepo::GetShader(const std::string& name) const {
  CGCHECK(shaders_.count(name) > 0) << "No shader name : " << name;
  ShaderLoadState* shader_load_state = &shaders_.at(name);
  if (shader_load_state->loaded == false) {
    CGLOG(ERROR) << "Loading Shading Begin : " << name;
    ShaderLoader shader_loader;
    shader_load_state->shader = shader_loader.Load(
        name,
        {{ShaderLoader::kVS, shader_load_state->config.has_vs_path() ? shader_load_state->config.vs_path() : ""},
         {ShaderLoader::kFS, shader_load_state->config.has_fs_path() ? shader_load_state->config.fs_path() : ""},
         {ShaderLoader::kGS, shader_load_state->config.has_gs_path() ? shader_load_state->config.gs_path() : ""},
         {ShaderLoader::kTS, shader_load_state->config.has_ts_path() ? shader_load_state->config.ts_path() : ""},
         {ShaderLoader::kCS, shader_load_state->config.has_cs_path() ? shader_load_state->config.cs_path() : ""}});
    CGLOG(ERROR) << "Loading Shading End : " << name;
    shader_load_state->loaded = true;
  }
  return shader_load_state->shader;
}

void ShaderProgramRepo::ReloadShaderPrograms() {
  for (auto& p : shaders_) {
    p.second.loaded = false;
  }
}

}  // namespace cg
