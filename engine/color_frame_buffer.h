#pragma once

#include <vector>

#include "engine/frame_buffer.h"
#include "engine/shader.h"

namespace engine {
class ColorFrameBuffer : public FrameBuffer {
 public:
  struct Option {
    std::string name;
    int width;
    int height;
    int mrt;
    std::vector<glm::vec4> clear_colors;
  };
  void Init(const Option& option);

  void OnBind() override;
  void Clear() override;
  void OnUnbind() override;

  Option option_;
};
}