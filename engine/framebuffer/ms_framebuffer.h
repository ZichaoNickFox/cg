#pragma once

#include <vector>

#include "engine/framebuffer/color_framebuffer.h"
#include "engine/framebuffer/framebuffer.h"
#include "engine/shader.h"

namespace engine {
class MSFramebuffer : public Framebuffer {
 public:
  struct Option {
    glm::ivec2 size;
    int mrt;
    std::vector<glm::vec4> clear_colors;
    int ms_num = 2;
  };
  void Init(const Option& option);

  void OnBind() override;
  void Clear() override;
  void OnUnbind() override;
  Texture GetColorTexture(int i = 0);
  Texture GetDepthTexture(int i = 0);

  void Blit(ColorFramebuffer* color_framebuffer);
  
 private:
  void CheckSupportMSNum(GLuint fbo, int num);

  Option option_;
};
}