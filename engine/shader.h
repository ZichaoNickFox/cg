#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "GL/glew.h"
#include "glm/glm.hpp"

namespace engine {
class Shader
{
public:
  Shader() {}
  Shader(const std::string& name, const std::string& vs, const std::string& fs,
         const std::string& gs = "", const std::string& ts = "");
  Shader& operator=(const Shader& other);

  void Use() const;
  void SetBool(const std::string &location_name, bool value) const;
  void SetInt(const std::string &location_name, int value) const;
  void SetFloat(const std::string &location_name, float value) const;
  void SetMat4(const std::string &location_name, const glm::mat4& value) const;
  void SetVec4(const std::string &location_name, const glm::vec4& value) const;
  void SetVec3(const std::string &location_name, const glm::vec3& value) const;
  GLuint id() const { return id_; }

private:
  void CheckCompileErrors(unsigned int shader, const std::string& type);
  GLint GetUniformLocation(const std::string& name) const;

 private:
  GLuint id_;
  std::string name_;
};
}