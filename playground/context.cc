#include "playground/context.h"

void Context::Init(const Config& config) {
  shader_repo_.Init(config);
  texture_repo_.Init(config);
}