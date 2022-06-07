#include "engine/SSBO.h"

#include "engine/gl.h"

namespace engine {

SSBO::SSBO() {
  glGenBuffers_(1, &ssbo_);
}

SSBO::~SSBO() {
  glDeleteBuffers_(1, &ssbo_);
}

};
