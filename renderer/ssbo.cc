#include "renderer/ssbo.h"

#include "renderer/gl.h"

namespace renderer {

SSBO::SSBO() {
  glGenBuffers_(1, &ssbo_);
}

SSBO::~SSBO() {
  glDeleteBuffers_(1, &ssbo_);
}

};
