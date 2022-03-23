#pragma once

#include "engine/frame_buffer/frame_buffer.h"

namespace engine{
class DepthFrameBuffer : public FrameBuffer {
 public:
  void Init(int Width, int Height) override;
  void Clear() override;
};
}