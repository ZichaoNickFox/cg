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

  Texture texture(int i = 0) const { return textures_[i]; }
  template<typename PixelType>
  std::shared_ptr<PixelType> GetTextureData(GLenum format, GLenum type) {
    std::shared_ptr<GLubyte> pixels(new PixelType[width_ * height_]);
    glReadPixels(0, 0, width_, height_, format, type, reinterpret_cast<void*>(*pixels));
    return pixels;
  }
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