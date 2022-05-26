#pragma once

#include "glm/glm.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

#include "engine/texture.h"
#include "engine/framebuffer_attachment.h"

namespace engine {
class Framebuffer {
public:
  struct Option {
    glm::ivec2 size;
    std::vector<FramebufferAttachment> attachments;
  };
  Framebuffer();
  ~Framebuffer();
  void Init(const Option& option);
  void Bind();
  void Unbind();
  void Clear();

  const glm::ivec2& size() { return option_.size; }
  GLuint fbo() { return fbo_; }

  Texture GetTexture(const std::string& name);
  void Blit(Framebuffer* framebuffer = nullptr);

 private:
  Option option_;
  GLuint fbo_;

  std::unordered_map<std::string, Texture> textures_;

  GLint resumption_fbo_;
  GLint resumption_viewport_[4];

  std::vector<GLuint> draw_buffers_;
  std::vector<glm::vec4> draw_buffers_clear_color_;
};
} // namespace engine