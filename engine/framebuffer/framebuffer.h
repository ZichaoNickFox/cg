#pragma once

#include "glm/glm.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

#include "engine/texture.h"

namespace engine {
class Framebuffer {
public:
  Framebuffer();
  virtual ~Framebuffer();
  void Bind();
  void Unbind();

  virtual void OnBind() = 0;
  virtual void Clear() = 0;
  virtual void OnUnbind() = 0;

  const glm::ivec2& size() { return size_; }
  GLuint fbo() { return fbo_; }

 protected:
  glm::ivec2 size_;
  GLuint fbo_;

  // vector for mrt
  std::vector<Texture> textures_;

  GLint resumption_fbo_;
  GLint resumption_viewport_[4];
};
};