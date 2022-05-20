#pragma once

#include "engine/debug.h"
#include "engine/framebuffer.h"

namespace engine{
class DepthFramebuffer : public Framebuffer {
 public:
  struct Option {
    glm::ivec2 size;
  };
  void Init(const Option& option);

  void OnBind();
  void Clear();
  void OnUnbind();
  Texture GetDepthTexture(int i = 0);
  glm::ivec2 size;
};
} // namespace engine