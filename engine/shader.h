#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "engine/debug.h"

namespace engine {
class Shader
{
public:
  Shader() {}  
  Shader(const std::string& name, const std::string& vs, const std::string& fs,
         const std::string& gs = "", const std::string& ts = ""/*, const std::vector<std:string>& depends*/);
  Shader& operator=(const Shader& other);

  void Use() const;
  void SetBool(const std::string &location_name, bool value) const;
  void SetInt(const std::string &location_name, int value) const;
  void SetFloat(const std::string &location_name, float value) const;
  void SetMat4(const std::string &location_name, const glm::mat4& value) const;
  void SetVec4(const std::string &location_name, const glm::vec4& value) const;
  void SetVec3(const std::string &location_name, const glm::vec3& value) const;
  GLuint id() const { return id_; }

  const std::string& name() { return name_; }

private:
  GLuint CompileShader(const std::string& code, GLuint shader_type);
  void LinkShader(GLuint program, const std::vector<GLuint>& objects);

  GLint GetUniformLocation(const std::string& name) const;
  GLuint id_;
  std::string name_;
};
}