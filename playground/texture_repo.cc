#include "playground/texture_repo.h"

#include <glog/logging.h>
#include "SOIL2/SOIL2.h"
#include "SOIL2/stb_image.h"

#include "playground/util.h"

using engine::Texture;

void TextureRepo::Init(const Config& config) {
  for (const TextureConfig& config : config.texture_config()) {
    State state;
    state.paths = util::ProtoRepeatedToVector(config.paths());
    state.texture_type = (Texture::Type)config.texture_type();
    textures_[config.name()] = state;
    LOG(ERROR) << "Init texture : " << config.name() << " (" << TextureType_Name(config.texture_type()) << ")";
  }
}

Texture TextureRepo::GetOrLoadTexture(const std::string& name) {
  BTCHECK(textures_.count(name) > 0) << "Cannot find texture : " << name;
  State* state = &textures_[name];
  if (!state->loaded) {
    if (state->texture_type == Texture::Texture2D) {
      BTCHECK(state->paths.size() == 1) << "Texture2D has 1 texture : " << name;
      state->texture = texture::LoadTexture2D(state->paths[0], true);
    } else if (state->texture_type == Texture::CubeMap) {
      state->texture = texture::LoadCubeMap(state->paths);
    } else {
      BTCHECK(false) << "Unsupported Texture Type";
    }
    state->loaded = true;
    LOG(ERROR) << "Loaded Texture : id - " << state->texture.id() << " | name - " << name;
  }
  return state->texture;
}

namespace texture {

Texture LoadTexture2D(const std::string& path_with_ext, bool useMipmap) {
  // OpenGL uv (0,0) is at left bottom
  // Texture uv (0,0) is at left top
  // So flip vertical uv
  stbi_set_flip_vertically_on_load(true);

  GLuint ret;
  glGenTextures(1, &ret);

  int SOILfmt;
  GLint internal_format;
  ParseImageFormat(path_with_ext, &SOILfmt, &internal_format);

  int width,height;
  unsigned char* image = SOIL_load_image(path_with_ext.c_str(), &width, &height, 0, SOILfmt);
  if (!image) {
    BTCHECK(false) << util::Format("cannot load image {}", path_with_ext);
  }
  glBindTexture(GL_TEXTURE_2D, ret);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, image);
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

Texture CreateFromData( GLubyte* data, int width, int height, bool useMipmap) {
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

bool VarifyChannel(const std::string& path, int channel) {
  std::string file_ext = util::GetFileExt(path);
  if (file_ext == "png") {
    if(channel != 1 && channel != 3 && channel != 4) {
      BTCHECK(false) << util::Format("current texture has {} channels while png need 1 or 3 or 4", channel);
      return false;
    }
    return channel == 1 || channel == 3 || channel == 4;
  } else if (file_ext == "jpg" || file_ext == "jpeg") {
    if(channel != 3) {
      BTCHECK(false) << util::Format("current texture has {} channels while jpg need 3", channel).c_str();
      return false;
    }
    return channel == 3;
  } else if (file_ext == "bmp") {
    return true;
  } else {
    BTCHECK(false) << util::Format("not support ext : {}", path);
    return false;
  }
  return false;
}

void RemoveFromGL(GLuint in) {
  glDeleteTextures(1, &in);
}

void GetInternalFormatSize(int internal_format, int* channel, int* format, int* type) {
  // GL Format
  // https://www.cs.uregina.ca/Links/class-info/315/WWW/Lab5/InternalFormats_OGL_Core_3_2.html
  switch(internal_format){
    case GL_DEPTH_COMPONENT: *channel = 1; *format = GL_DEPTH_COMPONENT; *type = GL_UNSIGNED_BYTE; return;
    case GL_DEPTH_STENCIL: *channel = 2; *format = GL_DEPTH_STENCIL; *type = GL_UNSIGNED_BYTE; return;
    case GL_RED: *channel = 1; *format = GL_RED; *type = GL_UNSIGNED_BYTE; return;
    case GL_RG: *channel = 2; *format = GL_RG;  *type = GL_UNSIGNED_BYTE; return;
    case GL_RGB: *channel = 3; *format = GL_RGB; *type = GL_UNSIGNED_BYTE; return;
    case GL_RGBA: *channel = 4; *format = GL_RGBA; *type = GL_UNSIGNED_BYTE; return;
    case GL_DEPTH_COMPONENT32F: *channel = 4; *format = GL_DEPTH_COMPONENT; *type = GL_FLOAT; return;
  }
  BTCHECK(false) << "Get Internal Format Size Failed : internal_format - " << internal_format;
}

void ParseImageFormat(const std::string& fileName, int* SOILfmt, GLint* internal_format) {
  // GL Format
  // https://community.khronos.org/t/gl-rgb-gl-rgba-gl-rgb8-what-is-what/41879
  std::string file_ext = util::GetFileExt(fileName);
  if(file_ext == "png") {
    *SOILfmt = SOIL_LOAD_RGBA;
    *internal_format = GL_RGBA;
    return;
  } else if (file_ext == "jpg" || file_ext == "jpeg") {
    *SOILfmt = SOIL_LOAD_RGB;
    *internal_format = GL_RGB;
    return;
  } else {
    BTCHECK(false) << "Unsupported texture format : " << file_ext;
  }
}

// failed return 0
void SaveTexture2D(const std::string& path_with_ext, GLuint tex) {
  glBindTexture(GL_TEXTURE_2D, tex);

  int width = -1, height = -1, internal_format = -1;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
  
  int channel = -1, format = -1, type = -1;
  GetInternalFormatSize(internal_format, &channel, &format, &type);
  GLubyte pixels[width * height * channel];
  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
  glGetTexImage(GL_TEXTURE_2D, 0, format, type, pixels);

  LOG(INFO) << "[Texture::saveTexture] path : " << path_with_ext;
  CHECK(width > 0) << "Widget must > 0";
  CHECK(height > 0) << "Height must > 0";

  BTCHECK(VarifyChannel(path_with_ext, channel)); 
	BTCHECK(SOIL_save_image(path_with_ext.c_str(), SOIL_SAVE_TYPE_BMP, width, height, channel, pixels));
}

Texture LoadCubeMap(const std::vector<std::string>& path) {
  GLuint textureId;
  glGenTextures(1, &textureId);
  
  Texture ret(textureId, Texture::CubeMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
  for(GLuint i =0 ; i < 6; ++i) {
    int SOILfmt;
    GLint internal_format;
    ParseImageFormat(path[i], &SOILfmt, &internal_format);
    int width, height;
    unsigned char* image = SOIL_load_image(path[i].c_str(), &width, &height, 0, SOILfmt);
    if(!image) {
      BTCHECK(false) << util::Format("cannot load image {}", path[i]);
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
      0, internal_format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, image
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

int SaveCubeMap(const std::vector<std::string>& path_with_exts, GLuint tex) {
  glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

  for(int i = 0; i < 6; ++i){
    int width = -1, height = -1, internal_format = -1;
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_HEIGHT, &height);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

    int channel = -1, format = -1, type = -1;
    GetInternalFormatSize(internal_format, &channel, &format, &type);
    GLubyte* pixels = new GLubyte[width * height * channel];

    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
    glGetTexImage(GL_TEXTURE_2D, 0, format, type, pixels);

    SaveTexture2D(path_with_exts[i], width, height, channel, pixels);
    delete[] pixels;
  }
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  return true;
}

Texture Copy(GLuint source) {
  glBindTexture(GL_TEXTURE_2D, source);
  int width = -1, height = -1, internal_format = -1;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

  int channel = -1, format = -1, type = -1;
  GetInternalFormatSize(internal_format, &channel, &format, &type);
  GLubyte* pixels = new GLubyte[width * height * channel];
  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
  glGetTexImage(GL_TEXTURE_2D, 0, format, type, pixels);

  GLuint ret;
  glGenTextures(1, &ret);
  glBindTexture(GL_TEXTURE_2D, ret);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  return {ret, Texture::Texture2D};
}
} // texture