#include "shader.h"

#include "glm/gtc/type_ptr.hpp"
#include <glog/logging.h>

namespace engine {

Shader& Shader::operator=(const Shader& other) {
  id_ = other.id_;
  name_ = other.name_;
  return *this;
}

Shader::Shader(const std::string& name, const std::string& vs, const std::string& fs) {
  name_ = name;

  unsigned int vertex, fragment;
  // vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  const char* vs_code = vs.data();
  const char* fs_code = fs.data();
  glShaderSource(vertex, 1, &vs_code, NULL);
  glCompileShader(vertex);
  CheckCompileErrors(vertex, "vertex");
  // fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fs_code, NULL);
  glCompileShader(fragment);
  CheckCompileErrors(fragment, "fragment");
  // shader Program
  id_ = glCreateProgram();
  glAttachShader(id_, vertex);
  glAttachShader(id_, fragment);
  glLinkProgram(id_);
  CheckCompileErrors(id_, "program");
  // delete the shaders as they're linked into our program now and no longer necessary
  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

void Shader::Use() const {
  glUseProgram(id_);
}

void Shader::SetBool(const std::string &location_name, bool value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform1i(location, (int)value);
}

void Shader::SetInt(const std::string &location_name, int value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform1i(location, value);
}

void Shader::SetFloat(const std::string &location_name, float value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform1f(location, value);
}

void Shader::SetMat4(const std::string &location_name, const glm::mat4& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

GLint Shader::GetUniformLocation(const std::string& location_name) const {
  GLint res = glGetUniformLocation(id_, location_name.c_str());
  if (res == -1 || res == GL_INVALID_VALUE || res == GL_INVALID_OPERATION) {
    CHECK(false) << "Cannot find uniform location '" << location_name << "' in shader '" << name_ << "'";
  }
  return res;
}

void Shader::CheckCompileErrors(unsigned int shader, const std::string& type) {
  int success;
  char info_log[1024];
  if (type != "program")
  {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(shader, 1024, NULL, info_log);
      CHECK(false) << type << " shader compile error : " << info_log;
    }
  }
  else
  {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetProgramInfoLog(shader, 1024, NULL, info_log);
      CHECK(false) << "Program link error :" << info_log;
    }
  }
}

}