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

  GLuint id() { return id_; }
  Type type(){return textureType_;}

 private:
  GLuint id_;
  Type textureType_ = Texture2D;
};

namespace texture {
  std::shared_ptr<engine::Texture> CreateTexture2D(int width, int height, int format);
  std::shared_ptr<engine::Texture> LoadTexture2D(const std::string& fileName, bool useMipmap = false);
  bool SaveTexture2D(const std::string& fullPath, GLuint tex);
  bool SaveTexture2D( const std::string& fullPath, int width, int height, int channels, const unsigned char *const data);
  
  std::shared_ptr<engine::Texture> LoadCubeMap(const std::vector<std::string>& path);
  int SaveCubeMap( const std::vector<std::string>& fullPaths, GLuint tex);

  void ParseImageFormat(const std::string& fileName, int* SOILfmt, GLint* internalFormat);
  bool VarifyChannel(const std::string& fileName, int channel);
  void RemoveFromGL(GLuint in);
  std::shared_ptr<engine::Texture> CreateFromData( GLubyte* data, int width, int height, bool useMipmap = false);

  int GetInternalFormatSize(int internalFormat);

  engine::Texture Copy(GLuint source);
}
}