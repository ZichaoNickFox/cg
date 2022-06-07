#pragma once

#include "engine/gl.h"
#include <glm/glm.hpp>

namespace engine {

class SSBO {
 public:
  SSBO();
  ~SSBO();

  template<typename DataStruct>
  void Init(int binding_point, const DataStruct& data);

  template<typename DataType>
  DataType GetData();
  int binding_point() { return binding_point_; }

 private:
  GLuint ssbo_;
  int binding_point_;
  bool inited_ = false;
};

template<typename DataStruct>
void SSBO::Init(int binding_point, const DataStruct& data) {
  binding_point_ = binding_point;
  glBindBuffer_(GL_SHADER_STORAGE_BUFFER, ssbo_);
  glBufferData_(GL_SHADER_STORAGE_BUFFER, sizeof(data), &data, GL_STREAM_COPY);
  glBindBufferBase_(GL_SHADER_STORAGE_BUFFER, binding_point, ssbo_);
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

};