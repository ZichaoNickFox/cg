#include "engine/automic_counter.h"

#include "engine/gl.h"

namespace engine {

AutomicCounter::AutomicCounter() {
  glGenBuffers_(1, &automic_counter_buffer_);
}

AutomicCounter::~AutomicCounter() {
  glDeleteBuffers_(1, &automic_counter_buffer_);
}

void AutomicCounter::Init(int binding_point) {
  binding_point_ = binding_point;
  glBindBuffer_(GL_ATOMIC_COUNTER_BUFFER, automic_counter_buffer_);
  glBufferData_(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
  glBindBuffer_(GL_ATOMIC_COUNTER_BUFFER, 0);
  inited_ = true;
}

void AutomicCounter::Reset(uint32_t value) {
  glBindBuffer_(GL_ATOMIC_COUNTER_BUFFER, automic_counter_buffer_);
  GLuint* ptr = (GLuint*)glMapBufferRange_(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
                                           GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
                                           | GL_MAP_UNSYNCHRONIZED_BIT);
  ptr[0] = value;
  glUnmapBuffer_(GL_ATOMIC_COUNTER_BUFFER);
  glBindBuffer_(GL_ATOMIC_COUNTER_BUFFER, 0);
}

};
