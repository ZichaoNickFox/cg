#pragma once

#include <unordered_map>

#include "engine/texture.h"
#include "playground/proto/config.pb.h"

class TextureRepo {
 public:
  void Init(const Config& config);
  engine::Texture GetOrLoadTexture(const std::string& name, bool flip_vertically = false, bool use_mipmap = false);
  void SaveTexture(const std::string& name, const engine::Texture& texture);
  void SaveCubemap(const std::string& name, int face, const engine::Texture& cubemap_face_texture2d);

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
  engine::Texture LoadTexture2D(const std::string& fileName, bool flip_vertically = true,
                                bool useMipmap = false, bool equirectangular = false);
  void SaveTexture2D(const std::string& fullPath, const engine::Texture& texture, bool multiple_sample = false);
  void SaveTexture2D(const std::string& fullPath, int width, int height, int channels, const unsigned char *const data);
  
  engine::Texture LoadCubeMap(const std::vector<std::string>& path);
  int SaveCubemap(const std::vector<std::string>& fullPaths, GLuint tex);

  void ParseImageFormat(const std::string& fileName, int* SOILfmt, GLint* internal_format);
  bool VarifyChannel(const std::string& fileName, int channel);
  void RemoveFromGL(GLuint in);

  engine::Texture CreateTexture2d(GLubyte* data, int width, int height, bool useMipmap = false);

  void GetInternalFormatSize(int internal_format, int* channel, int* format, int* type);

  engine::Texture Copy(GLuint source);
}