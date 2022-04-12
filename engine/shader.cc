#include "shader.h"

#include "glm/gtc/type_ptr.hpp"
#include <glog/logging.h>

#include "engine/debug.h"

namespace engine {

Shader& Shader::operator=(const Shader& other) {
  id_ = other.id_;
  name_ = other.name_;
  return *this;
}

Shader::Shader(const std::string& name, const std::string& vs, const std::string& fs,
               const std::string& gs, const std::string& ts) {
  bool have_gs = gs != "";
  bool have_ts = ts != "";
  name_ = name;

  unsigned int vertex, fragment, geometry, tessellation;
  const char* vs_code = vs.data();
  const char* fs_code = fs.data();
  const char* gs_code = gs.data();
  const char* ts_code = ts.data();

  // vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vs_code, NULL);
  glCompileShader(vertex);
  CheckCompileErrors(vertex, "vertex");

  // geometry shader
  if (have_gs) {
    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, &gs_code, NULL);
    glCompileShader(geometry);
    CheckCompileErrors(geometry, "geometry");
  }

  // fragment shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fs_code, NULL);
  glCompileShader(fragment);
  CheckCompileErrors(fragment, "fragment");

  // tessellation shader
  if (have_ts) {
    tessellation = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(tessellation, 1, &ts_code, NULL);
    glCompileShader(tessellation);
    CheckCompileErrors(tessellation, "tessellation");
  }
  
  // shader Program
  id_ = glCreateProgram();
  glAttachShader(id_, vertex);
  if (have_gs) {
    glAttachShader(id_, geometry);
  }
  if (have_ts) {
    glAttachShader(id_, tessellation);
  }
  glAttachShader(id_, fragment);
  glLinkProgram(id_);
  CheckCompileErrors(id_, "program");
  // delete the shaders as they're linked into our program now and no longer necessary
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  if (have_gs) {
    glDeleteShader(geometry);
  }
  if (have_ts) {
    glDeleteShader(tessellation);
  }
}

void Shader::Use() const {
  CGLOG(ERROR, false) << "Use shader : " << name_;
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

void Shader::SetVec4(const std::string &location_name, const glm::vec4& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform4fv(location, 1, (GLfloat*)&value);
}

void Shader::SetVec3(const std::string &location_name, const glm::vec3& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform3fv(location, 1, (GLfloat*)&value);
}

GLint Shader::GetUniformLocation(const std::string& location_name) const {
  GLint res = glGetUniformLocation(id_, location_name.c_str());
  if (res == -1 || res == GL_INVALID_VALUE || res == GL_INVALID_OPERATION) {
    CGCHECK(false) << "Cannot find uniform location '" << location_name << "' in shader '" << name_ << "'";
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
      std::string show_type;
      if (type == "vertex") show_type = "vs";
      else if (type == "fragment") show_type = "fs";
      else if (type == "geometry") show_type = "gs";
      else if (type == "tessellation") show_type = "ts";
      CGCHECK(false) << ": " << name_ << "." << show_type << " compile error : " << info_log;
    }
  }
  else
  {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetProgramInfoLog(shader, 1024, NULL, info_log);
      CGCHECK(false) << name_ << " : Program link error :" << info_log;
    }
  }
}

}