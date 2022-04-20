#include "shader.h"

#include "glm/gtc/type_ptr.hpp"
#include <glog/logging.h>
#include <map>

#include "engine/debug.h"

namespace engine {

Shader& Shader::operator=(const Shader& other) {
  id_ = other.id_;
  name_ = other.name_;
  return *this;
}

Shader::Shader(const std::string& name, const std::string& vs, const std::string& fs,
               const std::string& gs, const std::string& ts/*, const std::vector<std::string>& depends*/) {
  name_ = name;

  bool has_gs = gs != "";
  bool has_ts = ts != "";
  GLuint vertex_shader_object, fragment_shader_object, geometry_shader_object, tessellation_shader_object;
  vertex_shader_object = CompileShader(vs, GL_VERTEX_SHADER);
  fragment_shader_object = CompileShader(fs, GL_FRAGMENT_SHADER);
  if (has_gs) {
    geometry_shader_object = CompileShader(gs, GL_GEOMETRY_SHADER);
  }
  if (has_ts) {
    tessellation_shader_object = CompileShader(ts, GL_TESS_CONTROL_SHADER);
  }
/*
  for (const std::string& depend : depends) {
    const char* depend_code = depend.data();
    tessellation = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(tessellation, 1, &ts_code, NULL);
    glCompileShader(tessellation);
  }
*/

  id_ = glCreateProgram();
  std::vector<GLuint> objects{vertex_shader_object, fragment_shader_object};
  if (has_gs) objects.push_back(geometry_shader_object);
  if (has_ts) objects.push_back(tessellation_shader_object);
  LinkShader(id_, objects);
}

GLuint Shader::CompileShader(const std::string& code, GLuint shader_type) {
  GLuint object = glCreateShader(shader_type);
  const char* code_data = code.data();
  glShaderSource(object, 1, &code_data, NULL);
  glCompileShader(object);

  std::map<GLuint, std::string> shader_type_2_name = {
    {GL_VERTEX_SHADER, "vs"},
    {GL_FRAGMENT_SHADER, "fs"},
    {GL_GEOMETRY_SHADER, "gs"},
    {GL_TESS_CONTROL_SHADER, "ts"}
  };
  int success;
  char info_log[1024];
  glGetShaderiv(object, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(object, 1024, NULL, info_log);
    CGCHECK(false) << ": " << name_ << "." << shader_type_2_name[shader_type] << " compile error : " << info_log;
  }
  
  return object;
}

void Shader::LinkShader(GLuint program, const std::vector<GLuint>& objects) {

  for (GLuint object : objects) {
    glAttachShader(program, object);
  }
  glLinkProgram(program);

  int success;
  char info_log[1024];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(program, 1024, NULL, info_log);
    CGCHECK(false) << name_ << " : Program link error :" << info_log;
  }

  for (GLuint object : objects) {
    glDeleteShader(object);
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
}