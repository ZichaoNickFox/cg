#pragma once

#include <string>
#include "GL/glew.h"

namespace engine {

class Texture {
 public:
  enum Type{
    Texture2D,
    CubeMap
  };

  Texture() {}
  Texture(GLuint inid, Type in) : id_(inid), textureType_(in){}

  GLuint id() { return id_; }
  Type type(){return textureType_;}

 private:
  GLuint id_;
  Type textureType_ = Texture2D;
};

}