#pragma once

#include <vector>

#include "engine/framebuffer.h"
#include "engine/gl.h"
#include "engine/shader.h"

namespace engine {
class ColorFramebuffer : public Framebuffer {
 public:
  struct Option {
    glm::ivec2 size;
    int mrt;
    glm::vec4 clear_color;

    GLuint internal_format = GL_RGBA8;
    GLuint format = GL_RGBA;
    GLuint type = GL_UNSIGNED_BYTE; 
  };
  void Init(const Option& option);

  void OnBind();
  void Clear();
  void OnUnbind();
  Texture GetColorTexture(int i = 0);
  Texture GetDepthTexture();
  std::vector<GLubyte> GetColorTextureData(int i);
  
 private:
  Option option_;
  glm::ivec2 size_;
};
} // namespace engine