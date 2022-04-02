#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "engine/texture.h"

namespace engine {
class FrameBuffer {
public:
  virtual ~FrameBuffer() {}
  void Bind();
  void Unbind();

  virtual void OnBind() = 0;
  virtual void Clear() = 0;
  virtual void OnUnbind() = 0;
  virtual Texture GetTexture(int i = 0) = 0;

  int width() { return width_; }
  int height() { return height_; }

 protected:
  void SetFboNamePair(GLuint fbo, const std::string& name);

  int width_;
  int height_;
  GLuint fbo_;
  static std::unordered_map<GLuint, std::string> fbo_names_;

  // vector for mrt
  std::vector<Texture> textures_;

  GLint resumption_fbo_;
  GLint resumption_viewport_[4];

  std::string name_;
};
};