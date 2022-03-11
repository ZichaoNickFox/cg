#include "engine/texture_repo.h"

#include <glog/logging.h>
#include <SOIL2/SOIL2.h>
#include <SOIL2/stb_image.h>

#include "engine/file_util.h"
#include "engine/string_util.h"

namespace engine {
void TextureRepo::Init(const Config& config) {
  // OpenGL uv (0,0) is at left bottom
  // Texture uv (0,0) is at left top
  // So flip vertical uv
  stbi_set_flip_vertically_on_load(true);

  for (const TextureConfig& config : config.texture_config()) {
    State state;
    state.path = config.path();
    textures_[config.name()] = state;
  }
}

Texture TextureRepo::GetTexture(const std::string& name) {
  CHECK(textures_.count(name) > 0) << "Cannot find texture : " << name;
  State* state = &textures_[name];
  if (!state->loaded) {
    state->texture = LoadFromFile(state->path, true);
    state->loaded = true;
  }
  return state->texture;
}

Texture TextureRepo::LoadFromFile(const std::string& path_with_ext, bool useMipmap){
  GLuint ret;
  glGenTextures(1, &ret);

  int SOILfmt;
  GLint internalFormat;
  ParseImageFormat(path_with_ext, &SOILfmt, &internalFormat);

  int width,height;
  unsigned char* image = SOIL_load_image(path_with_ext.c_str(), &width, &height, 0, SOILfmt);
  if (!image) {
    CHECK(false) << string_util::Format("cannot load image {}", path_with_ext);
  }
  glBindTexture(GL_TEXTURE_2D, ret);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, image);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if(useMipmap){
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_MIPMAP);
  }else{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  SOIL_free_image_data(image);
  return Texture(ret, Texture::Texture2D);
}

Texture TextureRepo::CreateFromData( GLubyte* data, int width, int height, bool useMipmap) {
  GLuint ret;
  glGenTextures(1, &ret);
  glBindTexture(GL_TEXTURE_2D, ret);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if(useMipmap){
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_MIPMAP);
  }else{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  return Texture(ret, Texture::Texture2D);
}

bool TextureRepo::VarifyChannel(const std::string& path, int channel){
  std::string file_ext = file_util::GetFileExt(path);
  if (file_ext == "png") {
    if(channel != 1 && channel != 3 && channel != 4) {
      CHECK(false) << string_util::Format("current texture has {} channels while png need 1 or 3 or 4", channel);
      return false;
    }
    return channel == 1 || channel == 3 || channel == 4;
  } else if (file_ext == "jpg" || file_ext == "jpeg") {
    if(channel != 3) {
      CHECK(false) << string_util::Format("current texture has {} channels while jpg need 3", channel).c_str();
      return false;
    }
    return channel == 3;
  } else if (file_ext == "bmp") {
    return true;
  } else {
    CHECK(false) << string_util::Format("not support ext : {}", path);
    return false;
  }
  return false;
}

int TextureRepo::GetInternalFormatSize(int internalFormat) {
  // GL Format
  // https://community.khronos.org/t/gl-rgb-gl-rgba-gl-rgb8-what-is-what/41879
  switch(internalFormat){
    case GL_DEPTH_COMPONENT: return 1;
    case GL_DEPTH_STENCIL: return 2;
    case GL_RED: return 1;
    case GL_RG: return 2;
    case GL_RGB: return 3;
    case GL_RGBA: return 4;
    case GL_RGB8: return 3;
  }
  CHECK(false) << "Get Internal Format Size Failed";
  return -1;
}

void TextureRepo::ParseImageFormat(const std::string& fileName, int* SOILfmt, GLint* internalFormat) {
  // GL Format
  // https://community.khronos.org/t/gl-rgb-gl-rgba-gl-rgb8-what-is-what/41879
  std::string file_ext = file_util::GetFileExt(fileName);
  if(file_ext == "png") {
    *SOILfmt = SOIL_LOAD_RGBA;
    *internalFormat = GL_RGBA;
    return;
  } else if (file_ext == "jpg" || file_ext == "jpeg") {
    *SOILfmt = SOIL_LOAD_RGB;
    *internalFormat = GL_RGB;
    return;
  } else {
    CHECK(false) << "Unsupported texture format : " << file_ext;
  }
}

// failed return 0
bool TextureRepo::SaveToFile(const std::string& path_with_ext, int width,
                             int height, int channels, const unsigned char *const data) {
  LOG(INFO) << "[Texture::saveTexture] path : " << path_with_ext;
  CHECK(VarifyChannel(path_with_ext, channels)); 
	return SOIL_save_image (path_with_ext.c_str(), SOIL_SAVE_TYPE_BMP, width, height, channels, data);
}

bool TextureRepo::SaveToFile(const std::string& path_with_ext, GLuint tex) {
  glBindTexture(GL_TEXTURE_2D, tex);

  int width, height, internalFormat;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

  int channels = GetInternalFormatSize(internalFormat);
  GLubyte* pixels = new GLubyte[width * height * channels];
  glGetTexImage(GL_TEXTURE_2D, 0, internalFormat, GL_UNSIGNED_BYTE, pixels);

	bool ret = SaveToFile(path_with_ext, width, height, channels, pixels);
  delete[] pixels;
  CHECK(ret) << "Failed to save texture GLid:" << tex << " path:" << path_with_ext;
  return ret;
}

Texture TextureRepo::LoadCubeMap(const std::vector<std::string>& path) {
  GLuint textureId;
  glGenTextures(1, &textureId);
  
  Texture ret(textureId, Texture::CubeMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
  for(GLuint i =0 ; i < 6; ++i) {
    int SOILfmt;
    GLint internalFormat;
    ParseImageFormat(path[i], SOILfmt, internalFormat);
    int width, height;
    unsigned char* image = SOIL_load_image(path[i].c_str(), &width, &height, 0, SOILfmt);
    if(!image) {
      CHECK(false) << string_util::Format("cannot load image {}", path[i]);
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
      0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, image
    );
    SOIL_free_image_data(image);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  return ret;
}

int TextureRepo::SaveCubeMap(const std::vector<std::string>& path_with_exts, GLuint tex) {
  glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

  for(int i = 0; i < 6; ++i){
    int width, height, internalFormat;
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_HEIGHT, &height);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

    int channels = GetInternalFormatSize(internalFormat);
    GLubyte* pixels = new GLubyte[width * height * channels];
    glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, GL_UNSIGNED_BYTE, pixels);

    bool ret = SaveToFile(path_with_exts[i], width, height, channels, pixels);

    delete[] pixels;
    CHECK(ret) << "Failed to save cube map : " << path_with_exts[i];
  }
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  return true;
}

Texture TextureRepo::Copy(GLuint source){
  glBindTexture(GL_TEXTURE_2D, source);
  int width, height, internalFormat;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

  int channels = GetInternalFormatSize(internalFormat);
  GLubyte* pixels = new GLubyte[width * height * channels];
  glGetTexImage(GL_TEXTURE_2D, 0, internalFormat, GL_UNSIGNED_BYTE, pixels);

  GLuint ret;
  glGenTextures(1, &ret);
  glBindTexture(GL_TEXTURE_2D, ret);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  return {ret, Texture::Texture2D};
}
}