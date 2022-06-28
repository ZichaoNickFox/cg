#pragma once

#include "glm/glm.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "renderer/config.h"
#include "renderer/debug.h"
#include "renderer/texture.h"
#include "renderer/gl.h"

namespace renderer {
class ShaderProgram
{
 public:
  struct CodePart {
    std::string glsl_path;
    std::string code;
  };
  ShaderProgram() {}  
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
  GLuint id() const { return id_; }

  const std::string& name() { return name_; }

 private:
  std::string GetOneLineCompilerError430(const std::vector<ShaderProgram::CodePart>& code_parts, const std::string& gl_log);
  std::string GetMultipleLineCompileError(const std::vector<ShaderProgram::CodePart>& code_parts, const std::string& gl_log);
  GLuint Compile(const std::vector<CodePart>& code, GLuint shader_type);
  void Link(GLuint program, const std::vector<GLuint>& objects);

  GLint GetUniformLocation(const std::string& name) const;

  GLuint id_;
  std::string name_;
  mutable std::unordered_map<int, int> texture_2_unit_;
};

class ShaderProgramRepo {
 public:
  void Init(const Config& config);
  renderer::ShaderProgram GetShader(const std::string& name) const;
  void ReloadShaderPrograms();

 private:
  struct ShaderLoadState {
    ShaderLoadState(const ShaderConfig& in_config) { config = in_config; }
    bool loaded = false;
    renderer::ShaderProgram shader;
    ShaderConfig config;
  };
  mutable std::unordered_map<std::string, ShaderLoadState> shaders_;
};
}