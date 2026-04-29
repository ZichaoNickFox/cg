#include "renderer/shader_program.h"

#include "glog/logging.h"

#include "base/debug.h"
#include "renderer/shader_loader.h"

namespace cg {

void ShaderProgramBindings::Append(const ShaderProgramBindings& other) {
  bindings_.uniforms.insert(bindings_.uniforms.end(), other.bindings_.uniforms.begin(), other.bindings_.uniforms.end());
  bindings_.textures.insert(bindings_.textures.end(), other.bindings_.textures.begin(), other.bindings_.textures.end());
  bindings_.storage_textures.insert(bindings_.storage_textures.end(),
                                    other.bindings_.storage_textures.begin(),
                                    other.bindings_.storage_textures.end());
  bindings_.buffers.insert(bindings_.buffers.end(), other.bindings_.buffers.begin(), other.bindings_.buffers.end());
}

void ShaderProgramBindings::SetBool(const std::string& location_name, bool value) {
  bindings_.uniforms.push_back({location_name, value});
}

void ShaderProgramBindings::SetFloat(const std::string& location_name, float value) {
  bindings_.uniforms.push_back({location_name, value});
}

void ShaderProgramBindings::SetInt(const std::string& location_name, int value) {
  bindings_.uniforms.push_back({location_name, value});
}

void ShaderProgramBindings::SetMat4(const std::string& location_name, const glm::mat4& value) {
  bindings_.uniforms.push_back({location_name, value});
}

void ShaderProgramBindings::SetVec4(const std::string& location_name, const glm::vec4& value) {
  bindings_.uniforms.push_back({location_name, value});
}

void ShaderProgramBindings::SetVec3(const std::string& location_name, const glm::vec3& value) {
  bindings_.uniforms.push_back({location_name, value});
}

void ShaderProgramBindings::SetVec2(const std::string& location_name, const glm::vec2& value) {
  bindings_.uniforms.push_back({location_name, value});
}

void ShaderProgramBindings::SetTexture(const std::string& location_name, const Texture& value) {
  bindings_.textures.push_back({location_name, &value});
}

void ShaderProgramBindings::SetBufferBinding(const rhi::BufferBindingDesc& binding) {
  bindings_.buffers.push_back(binding);
}

void ShaderProgramBindings::SetStorageTexture(const std::string& location_name,
                                              const Texture& value,
                                              rhi::TextureAccess access) {
  bindings_.storage_textures.push_back({location_name, &value, access});
}

ShaderProgram::ShaderProgram(const ShaderProgramDesc& desc)
    : program_(rhi::GetDevice().CreateProgram({
          .name = desc.name,
          .kind = desc.kind,
          .vs = desc.vs,
          .fs = desc.fs,
          .gs = desc.gs,
          .ts = desc.ts,
          .cs = desc.cs,
      })),
      name_(desc.name) {}

const rhi::Program& ShaderProgram::ProgramRef() const {
  return *CGCHECK_NOTNULL(program_.get());
}

void ShaderProgram::ApplyBindings(const ShaderProgramBindings& bindings) const {
  ProgramRef().ApplyBindings(bindings.rhi_bindings());
}

void ShaderProgram::DrawBindings(const ShaderProgramBindings& bindings,
                                 const rhi::DrawDesc& desc) const {
  rhi::GetDevice().DrawBindings(ProgramRef(), bindings.rhi_bindings(), desc);
}

namespace {

void ValidateDispatchDesc(const rhi::ComputeDispatchDesc& desc) {
  CGCHECK(desc.workgroup_count.x > 0 && desc.workgroup_count.y > 0 && desc.workgroup_count.z > 0)
      << "Compute dispatch requires non-zero workgroup counts.";
}

}  // namespace

void ShaderProgram::DispatchComputeBindings(const ShaderProgramBindings& bindings,
                                            const rhi::ComputeDispatchDesc& desc) const {
  ValidateDispatchDesc(desc);
  rhi::GetDevice().DispatchComputeBindings(ProgramRef(), bindings.rhi_bindings(), desc);
}

void ShaderProgram::DispatchCompute(const rhi::ComputeDispatchDesc& desc) const {
  ValidateDispatchDesc(desc);
  rhi::GetDevice().DispatchComputeBindings(ProgramRef(), {}, desc);
}

void ShaderProgramRepo::Init(const Config& config) {
  config_ = &config;
  shaders_.clear();
}

ShaderProgram ShaderProgramRepo::GetShader(const std::string& name) const {
  CGCHECK(config_ != nullptr) << "ShaderProgramRepo::Init must be called before GetShader.";
  auto iter = shaders_.find(name);
  if (iter == shaders_.end()) {
    iter = shaders_.emplace(name, ShaderLoadState(config_->shader_config(name))).first;
  }
  ShaderLoadState* shader_load_state = &iter->second;
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
