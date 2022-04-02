#pragma once

#include "engine/debug.h"
#include "engine/frame_buffer.h"

namespace engine{
class DepthFrameBuffer : public FrameBuffer {
 public:
  struct Option {
    std::string name;
    int width;
    int height;
  };
  void Init(const Option& option);

  void OnBind() override;
  void Clear() override;
  void OnUnbind() override;
  Texture GetTexture(int i = 0) override;
};
}