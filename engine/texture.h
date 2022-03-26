#pragma once

#include <string>
#include "GL/glew.h"

namespace engine {
class Texture {
 public:
  enum Type{
    Texture2D = 0,
    CubeMap = 1
  };

  Texture() {}
  Texture(GLuint inid, Type in) : id_(inid), textureType_(in){}

  GLuint id() const { return id_; }
  GLuint* mutable_id() { return &id_; }
  Type type() const  {return textureType_;}

 private:
  GLuint id_;
  Type textureType_ = Texture2D;
};
}