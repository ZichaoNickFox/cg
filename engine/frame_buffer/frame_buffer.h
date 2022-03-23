#pragma once

#include <memory>

#include "engine/frame_buffer/frame_buffer_render.h"
#include "engine/texture.h"

namespace engine {
class FrameBuffer {
public:
  virtual ~FrameBuffer() {}
  virtual void Init(int inWidth, int inHeight) = 0;
  void Bind();
  void Unbind();
  virtual void Clear() = 0;

  std::shared_ptr<Texture> texture(){ return std::make_shared<Texture>(texture_, Texture::Texture2D); }
  std::shared_ptr<GLubyte> GetTextureData();
  int width() { return width_; }
  int height() { return height_; }

 protected:
  int width_;
  int height_;
  GLuint fbo_;
  GLuint texture_;

  GLint resumption_fbo_;
  GLint resumption_viewport_[4];
};
};