#pragma once

#include <vector>

#include "engine/frame_buffer/frame_buffer.h"
#include "engine/shader.h"

namespace engine {
class ColorFrameBuffer : public FrameBuffer {
 public:
  struct Option {
    glm::ivec2 size;
    int mrt;
    glm::vec4 clear_color;
  };
  void Init(const Option& option);

  void OnBind() override;
  void Clear() override;
  void OnUnbind() override;
  Texture GetColorTexture(int i = 0);
  Texture GetDepthTexture();
  std::vector<GLubyte> GetColorTextureData(int i);
  
 private:
  Option option_;
};
} // namespace engine