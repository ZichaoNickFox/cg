#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

#include "base/debug.h"
#include "renderer/config.h"
#include "rhi/device.h"
#include "renderer/texture.h"

namespace cg {
using ShaderCodePart = rhi::ShaderCodePart;

class ShaderProgramBindings {
 public:
  void Append(const ShaderProgramBindings& other);
  void SetBool(const std::string& location_name, bool value);
  void SetFloat(const std::string& location_name, float value);
  void SetInt(const std::string& location_name, int value);
  void SetMat4(const std::string& location_name, const glm::mat4& value);
  void SetVec4(const std::string& location_name, const glm::vec4& value);
  void SetVec3(const std::string& location_name, const glm::vec3& value);
  void SetVec2(const std::string& location_name, const glm::vec2& value);
  void SetTexture(const std::string& location_name, const Texture& value);
  void SetBufferBinding(const rhi::BufferBindingDesc& binding);
  void SetStorageTexture(const std::string& location_name,
                         const Texture& value,
                         rhi::TextureAccess access);

  const rhi::ProgramBindings& rhi_bindings() const { return bindings_; }

 private:
  rhi::ProgramBindings bindings_;
};

struct ShaderProgramDesc {
  std::string name;
  rhi::ProgramKind kind = rhi::ProgramKind::kRender;
  std::vector<ShaderCodePart> vs;
  std::vector<ShaderCodePart> fs;
  std::vector<ShaderCodePart> gs;
  std::vector<ShaderCodePart> ts;
  std::vector<ShaderCodePart> cs;
};

class ShaderProgram
{
 public:
  ShaderProgram() = default;
  explicit ShaderProgram(const ShaderProgramDesc& desc);

  void ApplyBindings(const ShaderProgramBindings& bindings) const;
  void DrawBindings(const ShaderProgramBindings& bindings,
                    const rhi::DrawDesc& desc) const;
  void DispatchComputeBindings(const ShaderProgramBindings& bindings,
                               const rhi::ComputeDispatchDesc& desc) const;
  void DispatchCompute(const rhi::ComputeDispatchDesc& desc) const;

  const std::string& name() const { return name_; }

 private:
  const rhi::Program& ProgramRef() const;

  std::shared_ptr<rhi::Program> program_;
  std::string name_;
};

class ShaderProgramRepo {
 public:
  void Init(const Config& config);
  cg::ShaderProgram GetShader(const std::string& name) const;
  void ReloadShaderPrograms();

 private:
  struct ShaderLoadState {
    explicit ShaderLoadState(const ShaderConfig& in_config) : config(in_config) {}
    bool loaded = false;
    cg::ShaderProgram shader;
    ShaderConfig config;
  };
  const Config* config_ = nullptr;
  mutable std::unordered_map<std::string, ShaderLoadState> shaders_;
};
}
