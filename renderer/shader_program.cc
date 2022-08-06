#include "shader.h"

#include <map>
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "base/debug.h"
#include "renderer/shader_loader.h"
#include "base/util.h"

namespace renderer {

ShaderProgram::ShaderProgram(const std::string& name, const std::vector<CodePart>& vs, const std::vector<CodePart>& fs,
               const std::vector<CodePart>& gs, const std::vector<CodePart>& ts) {
  name_ = name;

  bool has_gs = gs.size() > 0;
  bool has_ts = ts.size() > 0;
  GLuint vertex_shader_object, fragment_shader_object, geometry_shader_object, tessellation_shader_object;
  vertex_shader_object = Compile(vs, GL_VERTEX_SHADER);
  fragment_shader_object = Compile(fs, GL_FRAGMENT_SHADER);
  if (has_gs) {
    geometry_shader_object = Compile(gs, GL_GEOMETRY_SHADER);
  }
  if (has_ts) {
    tessellation_shader_object = Compile(ts, GL_TESS_CONTROL_SHADER);
  }

  id_ = glCreateProgram_();
  std::vector<GLuint> objects{vertex_shader_object, fragment_shader_object};
  if (has_gs) objects.push_back(geometry_shader_object);
  if (has_ts) objects.push_back(tessellation_shader_object);
  Link(id_, objects);
}

ShaderProgram::ShaderProgram(const std::string& name, const std::vector<CodePart>& cs) {
  name_ = name;

  id_ = glCreateProgram_();
  GLuint compute_shader_object = 0;
  compute_shader_object = Compile(cs, GL_COMPUTE_SHADER);
  std::vector<GLuint> objects{compute_shader_object};
  Link(id_, objects);
}

namespace {
// TODO : use util's
bool StartsWith(const std::string& str, const std::string& start_with) {
  return str.rfind(start_with, 0) == 0;
}
} 

std::string ShaderProgram::GetOneLineCompilerError430(const std::vector<ShaderProgram::CodePart>& code_parts,
                                                      const std::string& gl_log) {
  std::string res = gl_log;
  int source_line_num = 0;
  const std::string starting = "0(";
  res.erase(0, starting.size());

  int line_num_end = res.find_first_of(")");
  source_line_num = std::atoi(res.substr(0, line_num_end).c_str());
  res.erase(0, line_num_end);

  const std::string next = ") ";
  res.erase(0, next.size());

  int line_sum = 1;
  for (const ShaderProgram::CodePart& code_part : code_parts) {
    int line_num_in_file = 1;
    for (char c : code_part.code) {
      if (c == '\n') {
        line_sum++;
        line_num_in_file ++;
        if (line_sum == source_line_num) {
          return std::format("\"{}\" {} {}", code_part.glsl_path, line_num_in_file, res);
        }
      }
    }
  }
  CGKILL("Cannot find a file? source_line_num ~ ") << name_ << " | " << gl_log;
  return "";
}

std::string ShaderProgram::GetMultipleLineCompileError(const std::vector<ShaderProgram::CodePart>& code_parts,
                                                       const std::string& gl_log) {
  std::vector<std::string> error_logs;
  std::string one_error;
  for (const char c : gl_log) {
    if (c == '\n') {
      error_logs.push_back(one_error);
      one_error = "";
    } else {
      one_error += c;
    }
  }
  std::string res = "\n";
  for (const std::string& one_error : error_logs) {
    res += GetOneLineCompilerError430(code_parts, one_error);
    res += "\n";
  }
  return res;
}

GLuint ShaderProgram::Compile(const std::vector<CodePart>& code_parts, GLuint shader_type) {
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
    std::string compile_error_info(info_log);
    CGKILL("Shader Compile Error : name~") << name_ << GetMultipleLineCompileError(code_parts, compile_error_info);
  }
  
  return object;
}

void ShaderProgram::Link(GLuint program, const std::vector<GLuint>& objects) {
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

void ShaderProgram::Use() const {
  CGCHECK(glIsProgram_(id_)) << "glIsProgram failed, glCreateProgram? not glDeleteProgram? id ~ " << id_;
  glUseProgram_(id_);

  texture_2_unit_.clear();
}

void ShaderProgram::SetBool(const std::string &location_name, bool value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform1i_(location, (int)value);
}

void ShaderProgram::SetInt(const std::string &location_name, int value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform1i_(location, value);
}

void ShaderProgram::SetFloat(const std::string &location_name, float value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform1f_(location, value);
}

int ShaderProgram::SetTexture(const std::string &location_name, const Texture& value) const {
  CGCHECK(value.Varify());
  int unit = -1;
  if (texture_2_unit_.count(value.id()) > 0) {
    unit = texture_2_unit_.at(value.id());
  } else {
    unit = texture_2_unit_.size();
    CGCHECK(unit <= 31) << "Only TEXTURE0 - TEXTURE31 supported : " << unit;
    texture_2_unit_[value.id()] = unit;
  }
  SetInt(location_name, unit);
  glActiveTexture_(GL_TEXTURE0 + unit);
  if (value.meta().type == Texture::kTexture2D) {
    glBindTexture_(GL_TEXTURE_2D, value.id());
  } else if (value.meta().type == Texture::kCubemap) {
    glBindTexture_(GL_TEXTURE_CUBE_MAP, value.id());
  } else if (value.meta().type == Texture::kTexture2DArray) {
    glBindTexture_(GL_TEXTURE_2D_ARRAY, value.id());
  }
  return unit;
}

void ShaderProgram::SetMat4(const std::string &location_name, const glm::mat4& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniformMatrix4fv_(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::SetVec4(const std::string &location_name, const glm::vec4& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform4fv_(location, 1, glm::value_ptr(value));
}

void ShaderProgram::SetVec3(const std::string &location_name, const glm::vec3& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform3fv_(location, 1, glm::value_ptr(value));
}

void ShaderProgram::SetVec2(const std::string &location_name, const glm::vec2& value) const {
  GLint location = GetUniformLocation(location_name);
  glUniform2fv_(location, 1, glm::value_ptr(value));
}

GLint ShaderProgram::GetUniformLocation(const std::string& location_name) const {
  GLint res = glGetUniformLocation_(id_, location_name.c_str());
  if (res == GL_INVALID_VALUE || res == GL_INVALID_OPERATION) {
    CGKILL("GetUniformLocation Failed") << res;
  } else if (res == -1) {
    // It's OK, like passing camera to glsl, but near / far not be used.
  }
  return res;
}

void ShaderProgramRepo::Init(const Config& config) {
  std::string content;
  for (const auto& p : config.shader_configs()) {
    const ShaderConfig& shader_config = p.second;
    std::string name = shader_config.name();
    shaders_.insert(std::make_pair(name, ShaderLoadState(shader_config)));
    CGLOG(ERROR) << "Init shader : " << name;
  }
}

ShaderProgram ShaderProgramRepo::GetShader(const std::string& name) const {
  CGCHECK(shaders_.count(name) > 0) << "No shader name : " << name;
  ShaderLoadState* shader_load_state = &shaders_.at(name);
  if (shader_load_state->loaded == false) {
    CGLOG(ERROR) << "Loading Shading Begin : " << name;
    ShaderLoader shader_loader;
    shader_load_state->shader = shader_loader.Load(name, {
        {ShaderLoader::kVS, shader_load_state->config.has_vs_path() ? shader_load_state->config.vs_path() : ""},
        {ShaderLoader::kFS, shader_load_state->config.has_fs_path() ? shader_load_state->config.fs_path() : ""},
        {ShaderLoader::kGS, shader_load_state->config.has_gs_path() ? shader_load_state->config.gs_path() : ""},
        {ShaderLoader::kTS, shader_load_state->config.has_ts_path() ? shader_load_state->config.ts_path() : ""},
        {ShaderLoader::kCS, shader_load_state->config.has_cs_path() ? shader_load_state->config.cs_path() : ""}});
    CGLOG(ERROR) << "Loading Shading End : " << name;
    shader_load_state->loaded = true;
  }
  return shader_load_state->shader;
}

void ShaderProgramRepo::ReloadShaderPrograms() {
  for (auto& p : shaders_) {
    p.second.loaded = false;
  }
}
} // namespace renderer