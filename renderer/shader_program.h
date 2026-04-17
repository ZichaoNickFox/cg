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
class ShaderProgram
{
 public:
  using CodePart = rhi::ShaderCodePart;

  ShaderProgram() = default;
  // Render Shader
  ShaderProgram(const std::string& name, const std::vector<CodePart>& vs, const std::vector<CodePart>& fs,
         const std::vector<CodePart>& gs, const std::vector<CodePart>& ts);
  // Compute Shader
  ShaderProgram(const std::string& name, const std::vector<CodePart>& cs);

  void Use() const;
  void SetBool(const std::string &location_name, bool value) const;
  void SetFloat(const std::string &location_name, float value) const;
  void SetInt(const std::string &location_name, int value) const;
  int SetTexture(const std::string& location_name, const Texture& value) const;
  void SetMat4(const std::string &location_name, const glm::mat4& value) const;
  void SetVec4(const std::string &location_name, const glm::vec4& value) const;
  void SetVec3(const std::string &location_name, const glm::vec3& value) const;
  void SetVec2(const std::string &location_name, const glm::vec2& value) const;
  uint32_t id() const;

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
    ShaderLoadState(const ShaderConfig& in_config) { config = in_config; }
    bool loaded = false;
    cg::ShaderProgram shader;
    ShaderConfig config;
  };
  mutable std::unordered_map<std::string, ShaderLoadState> shaders_;
};
}
