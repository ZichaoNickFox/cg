#pragma once

#include <vector>

#include "engine/frame_buffer.h"
#include "engine/shader.h"

namespace engine {
class GBuffer : public FrameBuffer {
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

  GLuint fbo() { return fbo_; }
  
 private:
  Option option_;
};
}