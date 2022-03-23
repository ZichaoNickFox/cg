#pragma once

#include "engine/frame_buffer/frame_buffer.h"
#include "engine/shader.h"

namespace engine {
class ColorFrameBuffer : public FrameBuffer {
 public:
  void Init(int inWidth, int inHeight) override;
  void Clear() override;

 private:
  GLuint rbo_;
};
}