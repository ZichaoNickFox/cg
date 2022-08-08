#pragma once

#include "renderer/definition.h"
#include "renderer/gl.h"
#include <glm/glm.hpp>

namespace cg {

class SSBO {
 public:
  SSBO(int binding_point) : binding_point_(binding_point) { glGenBuffers_(1, &ssbo_); }
  ~SSBO() { glDeleteBuffers_(1, &ssbo_); }

  template<typename DataStruct>
  void SetData(int size_in_byte, const DataStruct* data);

  template<typename DataType>
  DataType GetData();

  int binding_point() { return binding_point_; }

 private:
  GLuint ssbo_;
  int binding_point_;
  bool inited_ = false;
};

template<typename DataStruct>
void SSBO::SetData(int size_in_byte, const DataStruct* data) {
  glBindBuffer_(GL_SHADER_STORAGE_BUFFER, ssbo_);
  glBufferData_(GL_SHADER_STORAGE_BUFFER, size_in_byte, data, GL_STREAM_COPY);
  if (!inited_) {
    glBindBufferBase_(GL_SHADER_STORAGE_BUFFER, binding_point_, ssbo_);
    inited_ = true;
  }
  glBindBuffer_(GL_SHADER_STORAGE_BUFFER, 0);
}

template<typename DataStruct>
DataStruct SSBO::GetData() {
  glBindBuffer_(GL_SHADER_STORAGE_BUFFER, ssbo_);
  void* ptr = CGCHECK_NOTNULL(glMapBuffer_(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));
  DataStruct data_ptr = *CGCHECK_NOTNULL(static_cast<DataStruct*>(ptr));
  glUnmapBuffer_(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer_(GL_SHADER_STORAGE_BUFFER, 0);
  return data_ptr;
}

} // namespace cg