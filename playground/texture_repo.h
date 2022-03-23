#pragma once

#include <unordered_map>

#include "engine/texture.h"
#include "playground/config.pb.h"

class TextureRepo {
 public:
  void Init(const Config& config);
  engine::Texture GetOrLoadTexture(const std::string& name);

 private:
  struct State {
    engine::Texture texture;
    bool loaded = false;
    std::vector<std::string> paths;
    engine::Texture::Type texture_type;
  };
  std::unordered_map<std::string, State> textures_;
};

namespace texture {
  engine::Texture LoadTexture2D(const std::string& fileName, bool useMipmap = false);
  bool SaveTexture2D(const std::string& fullPath, GLuint tex);
  bool SaveTexture2D( const std::string& fullPath, int width, int height, int channels, const unsigned char *const data);
  
  engine::Texture LoadCubeMap(const std::vector<std::string>& path);
  int SaveCubeMap( const std::vector<std::string>& fullPaths, GLuint tex);

  void ParseImageFormat(const std::string& fileName, int* SOILfmt, GLint* internalFormat);
  bool VarifyChannel(const std::string& fileName, int channel);
  void RemoveFromGL(GLuint in);
  engine::Texture CreateFromData( GLubyte* data, int width, int height, bool useMipmap = false);

  int GetInternalFormatSize(int internalFormat);

  engine::Texture Copy(GLuint source);
}