#include "shader.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include <map>

#include "engine/debug.h"

namespace engine {
Shader& Shader::operator=(const Shader& other) {
  id_ = other.id_;
  name_ = other.name_;
  return *this;
}

Shader::Shader(const std::string& name, const std::vector<CodePart>& vs, const std::vector<CodePart>& fs,
               const std::vector<CodePart>& gs, const std::vector<CodePart>& ts) {
  name_ = name;

  bool has_gs = gs.size() > 0;
  bool has_ts = ts.size() > 0;
  GLuint vertex_shader_object, fragment_shader_object, geometry_shader_object, tessellation_shader_object;
  vertex_shader_object = CompileShader(vs, GL_VERTEX_SHADER);
  fragment_shader_object = CompileShader(fs, GL_FRAGMENT_SHADER);
  if (has_gs) {
    geometry_shader_object = CompileShader(gs, GL_GEOMETRY_SHADER);
  }
  if (has_ts) {
    tessellation_shader_object = CompileShader(ts, GL_TESS_CONTROL_SHADER);
  }

  id_ = glCreateProgram_();
  std::vector<GLuint> objects{vertex_shader_object, fragment_shader_object};
  if (has_gs) objects.push_back(geometry_shader_object);
  if (has_ts) objects.push_back(tessellation_shader_object);
  LinkShader(id_, objects);
}

namespace {
// TODO : use util's
bool StartsWith(const std::string& str, const std::string& start_with) {
  return str.rfind(start_with, 0) == 0;
}
void CorrectCompileMessage(const std::vector<Shader::CodePart>& code_parts, std::string* gl_log) {
  const std::string starting = "ERROR: 0:";
  int source_line_num = 0;
  if (StartsWith(*gl_log, starting)) {
    gl_log->erase(0, starting.size());
    int line_end = gl_log->find_first_of(":");
    std::string str_line_num = gl_log->substr(0, line_end);
    source_line_num = std::atoi(str_line_num.c_str());
    gl_log->erase(0, line_end);
  }
  int line_sum = 1;
  for (const Shader::CodePart& code_part : code_parts) {
    int line_num_in_file = 1;
    for (char c : code_part.code) {
      if (c == '\n') {
        line_sum++;
        line_num_in_file ++;
        if (line_sum == source_line_num) {
          *gl_log = code_part.glsl_path + " " + std::to_string(line_num_in_file) + *gl_log;
          LOG(ERROR) << *gl_log;
          return;
        }
      }
    }
  }
} 
}
GLuint Shader::CompileShader(const std::vector<CodePart>& code_parts, GLuint shader_type) {
  GLuint object = glCreateShader_(shader_type);
   
  std::vector<const char*> code_data(code_parts.size());
  for (int i = 0; i < code_parts.size(); ++i) {
    const CodePart& code_part = code_parts[i];
    code_data[i] = code_part.code.data();
  }

  glShaderSource_(object, code_parts.size(), code_data.data(), NULL);
  glCompileShader_(object);

  int success;
  char info_log[1024];
  glGetShaderiv_(object, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog_(object, 1024, NULL, info_log);
    std::string correct_compile_message(info_log);
    CorrectCompileMessage(code_parts, &correct_compile_message);
    CGCHECK(false) << name_ << " compile error : " << correct_compile_message;
  }
  
  return object;
}

void Shader::LinkShader(GLuint program, const std::vector<GLuint>& objects) {
  for (GLuint object : objects) {
    glAttachShader_(program, object);
  }
  glLinkProgram_(program);

  int success;
  char info_log[1024];
  glGetProgramiv_(program, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog_(program, 1024, NULL, info_log);
    CGCHECK(false) << name_ << " : Program link error :" << info_log;
  }

  for (GLuint object : objects) {
    glDeleteShader_(object);
  }
}

void Shader::Use() const {
  glUseProgram_(id_);
}

void Shader::SetBool(const std::string &location_name, bool value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform1i_(location, (int)value);
}

void Shader::SetInt(const std::string &location_name, int value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform1i_(location, value);
}

void Shader::SetFloat(const std::string &location_name, float value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform1f_(location, value);
}

void Shader::SetMat4(const std::string &location_name, const glm::mat4& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniformMatrix4fv_(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string &location_name, const glm::vec4& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform4fv_(location, 1, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string &location_name, const glm::vec3& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform3fv_(location, 1, glm::value_ptr(value));
}

void Shader::SetVec2(const std::string &location_name, const glm::vec2& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform2fv_(location, 1, glm::value_ptr(value));
}

GLint Shader::GetUniformLocation(const std::string& location_name) const {
  GLint res = glGetUniformLocation_(id_, location_name.c_str());
  if (res == -1 || res == GL_INVALID_VALUE || res == GL_INVALID_OPERATION) {
    CGLOG(ERROR) << "Cannot find uniform location '" << location_name << "' in shader '" << name_ << "'";
  }
  return res;
}

void Shader::Clear() {
  // TODO clear all uniforms
}
}