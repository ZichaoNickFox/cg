#pragma once

#include <string>
#include "GL/glew.h"

namespace engine {
class Texture {
 public:
  enum Type{
    Texture2D = 0,
    Cubemap = 1,
  };

  Texture() {}
  Texture(GLuint inid, Type in) : id_(inid), textureType_(in){}
  Texture(GLuint inid, Type in, const std::string& info) : id_(inid), textureType_(in), info_(info) {}

  GLuint id() const { return id_; }
  GLuint* mutable_id() { return &id_; }
  Type type() const  {return textureType_;}
  void SetInfo(const std::string& info) { info_ = info; }
  std::string info() const { return info_; }

 private:
  GLuint id_;
  Type textureType_ = Texture2D;
  std::string info_;
};
}