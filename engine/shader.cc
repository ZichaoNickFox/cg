#include "shader.h"

#include <glog/logging.h>

namespace engine {

Shader& Shader::operator=(const Shader& other) {
  id_ = other.id_;
  return *this;
}

Shader::Shader(const std::string& vs, const std::string& fs)
{
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

void Shader::Use() {
  glUseProgram(id_);
}

void Shader::SetBool(const std::string &name, bool value) const
{
  glUniform1i(glGetUniformLocation(id_, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string &name, int value) const
{
  glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}

void Shader::SetFloat(const std::string &name, float value) const
{
  glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}

void Shader::CheckCompileErrors(unsigned int shader, const std::string& type)
{
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