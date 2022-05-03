#pragma once

#include "engine/debug.h"
#include "engine/framebuffer/framebuffer.h"

namespace engine{
class DepthFramebuffer : public Framebuffer {
 public:
  struct Option {
    glm::ivec2 size;
  };
  void Init(const Option& option);

  void OnBind() override;
  void Clear() override;
  void OnUnbind() override;
  Texture GetDepthTexture(int i = 0);
};
} // namespace engine