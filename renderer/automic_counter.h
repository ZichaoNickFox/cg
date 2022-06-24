#pragma once

#include "renderer/gl.h"
#include <glm/glm.hpp>

namespace renderer {

class AutomicCounter {
 public:
  AutomicCounter();
  ~AutomicCounter();

  void Init(int binding_point);
  int binding_point() { return binding_point_; }
  void Reset(uint32_t value);

 private:
  GLuint automic_counter_buffer_;
  int binding_point_;
  bool inited_ = false;
};

};