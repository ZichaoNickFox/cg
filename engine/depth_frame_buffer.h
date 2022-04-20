#pragma once

#include "engine/debug.h"
#include "engine/frame_buffer.h"

namespace engine{
class DepthFrameBuffer : public FrameBuffer {
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
}