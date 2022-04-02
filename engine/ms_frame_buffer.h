#pragma once

#include <vector>

#include "engine/color_frame_buffer.h"
#include "engine/frame_buffer.h"
#include "engine/shader.h"

namespace engine {
class MSFrameBuffer : public FrameBuffer {
 public:
  struct Option {
    std::string name;
    int width;
    int height;
    int mrt;
    std::vector<glm::vec4> clear_colors;
    int ms_num = 2;
  };
  void Init(const Option& option);

  void OnBind() override;
  void Clear() override;
  void OnUnbind() override;
  Texture GetTexture(int i = 0) override;

  void Blit(ColorFrameBuffer* color_frame_buffer);
  
 private:
  void CheckSupportMSNum(GLuint fbo, int num);

  Option option_;
};
}