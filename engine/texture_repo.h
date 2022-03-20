#pragma once

#include <unordered_map>

#include "engine/config.pb.h"
#include "engine/texture.h"

namespace engine {
class TextureRepo {
 public:
  void Init(const Config& config);

  std::shared_ptr<Texture> GetOrLoadTexture(const std::string& name);
  bool SaveTexture2D(const std::string& fullPath, GLuint tex);

 private:
  std::shared_ptr<Texture> LoadTexture2D(const std::string& fileName, bool useMipmap = false);
  bool SaveTexture2D( const std::string& fullPath, int width, int height, int channels, const unsigned char *const data);
  
  std::shared_ptr<Texture> LoadCubeMap(const std::vector<std::string>& path);
  int SaveCubeMap( const std::vector<std::string>& fullPaths, GLuint tex);

  void ParseImageFormat(const std::string& fileName, int* SOILfmt, GLint* internalFormat);
  bool VarifyChannel(const std::string& fileName, int channel);
  void RemoveFromGL(GLuint in){ glDeleteTextures(1, &in); }
  Texture CreateFromData( GLubyte* data, int width, int height, bool useMipmap = false);

  int GetInternalFormatSize(int internalFormat);

  Texture Copy(GLuint source);

  struct State {
    std::shared_ptr<Texture> texture;
    bool loaded = false;
    std::vector<std::string> paths;
    Texture::Type texture_type;
  };
  std::unordered_map<std::string, State> textures_;
};
}