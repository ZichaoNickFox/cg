#include "playground/texture_repo.h"

#include <glog/logging.h>
#include "SOIL2/SOIL2.h"
#include "SOIL2/stb_image.h"

#include "engine/debug.h"
#include "playground/util.h"

using engine::Texture;

void TextureRepo::Init(const Config& config) {
  for (const TextureConfig& config : config.texture_config()) {
    State state;
    state.paths = util::ProtoRepeatedToVector(config.paths());
    state.texture_type = (Texture::Type)config.texture_type();
    textures_[config.name()] = state;
    CGLOG(ERROR) << "Init texture : " << config.name() << " (" << TextureType_Name(config.texture_type()) << ")";
  }
}

Texture TextureRepo::GetOrLoadTexture(const std::string& name, bool flip_vertically, bool use_mipmap) {
  CGCHECK(textures_.count(name) > 0) << "Cannot find texture : " << name;
  State* state = &textures_[name];
  if (!state->loaded) {
    if (state->texture_type == Texture::Texture2D) {
      CGCHECK(state->paths.size() == 1) << "Texture2D has 1 texture : " << name;
      state->texture = texture::LoadTexture2D(state->paths[0], flip_vertically, use_mipmap);
    } else if (state->texture_type == Texture::CubeMap) {
      state->texture = texture::LoadCubeMap(state->paths);
    } else {
      CGCHECK(false) << "Unsupported Texture Type";
    }
    state->loaded = true;
    CGLOG(ERROR) << "Loaded Texture : id - " << state->texture.id() << " | name - " << name;
  }
  return state->texture;
}

namespace texture {

Texture LoadTexture2D(const std::string& path_with_ext, bool flip_vertically, bool useMipmap) {
  // OpenGL uv (0,0) is at left bottom
  // Texture uv (0,0) is at left top
  // So flip vertical uv
  stbi_set_flip_vertically_on_load(flip_vertically);

  GLuint id;
  glGenTextures(1, &id);

  int SOILfmt;
  GLint internal_format;
  ParseImageFormat(path_with_ext, &SOILfmt, &internal_format);

  int width,height;
  unsigned char* image = SOIL_load_image(path_with_ext.c_str(), &width, &height, 0, SOILfmt);
  if (!image) {
    CGCHECK(false) << util::Format("cannot load image {}", path_with_ext);
  }
  glBindTexture(GL_TEXTURE_2D, id);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, image);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if(useMipmap){
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }else{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  SOIL_free_image_data(image);
  return Texture(id, Texture::Texture2D, util::FileName(path_with_ext));
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
      CGCHECK(false) << util::Format("current texture has {} channels while png need 1 or 3 or 4", channel);
      return false;
    }
    return channel == 1 || channel == 3 || channel == 4;
  } else if (file_ext == "jpg" || file_ext == "jpeg") {
    if(channel != 3) {
      CGCHECK(false) << util::Format("current texture has {} channels while jpg need 3", channel).c_str();
      return false;
    }
    return channel == 3;
  } else if (file_ext == "bmp") {
    return true;
  } else {
    CGCHECK(false) << util::Format("not support ext : {}", path);
    return false;
  }
  return false;
}

void RemoveFromGL(GLuint in) {
  glDeleteTextures(1, &in);
}

void GetInternalFormatSize(int internal_format, int* channel, int* byte_per_channel, int* format, int* type) {
  struct Info {
    int channel;
    int byte_per_channel;
    int format;
    int type;
    std::string log;
  };
  // GL Format
  // https://www.cs.uregina.ca/Links/class-info/315/WWW/Lab5/InternalFormats_OGL_Core_3_2.html
  std::unordered_map<GLuint, Info> info_map = {
    {GL_DEPTH_COMPONENT, {1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, "GL_DEPTH_COMPONENT"}},
    {GL_DEPTH_STENCIL, {2, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, "GL_DEPTH_STENCIL"}},
    {GL_RED, {1, 1, GL_RED, GL_UNSIGNED_BYTE, "GL_RED"}},
    {GL_RG, {2, 2, GL_RG, GL_UNSIGNED_BYTE, "GL_RG"}},
    {GL_RGB, {2, 2, GL_RGB, GL_UNSIGNED_BYTE, "GL_RGB"}},
    {GL_RGB32F, {3, 4, GL_RGB, GL_FLOAT, "GL_RGB32F"}},
    {GL_RGB8, {3, 1, GL_RGB, GL_UNSIGNED_BYTE, "GL_RGB8"}},
    {GL_RGBA, {4, 4, GL_RGBA, GL_UNSIGNED_BYTE, "GL_RGBA"}},
    {GL_RGBA8, {4, 1, GL_RGBA, GL_UNSIGNED_BYTE, "GL_RGBA8"}},
    {GL_RGBA32F, {4, 4, GL_RGBA, GL_FLOAT, "GL_RGBA32F"}},
    {GL_DEPTH_COMPONENT32F, {4, 4, GL_DEPTH_COMPONENT, GL_FLOAT, "GL_DEPTH_COMPONENT32F"}},
  };
  CGCHECK(info_map.count(internal_format) > 0)
      << "Get Internal Format Size Failed : internal_format - " << internal_format;
  const Info info = info_map[internal_format];
  LOG(ERROR) << "[GetInternalFormatSize] " << info.log;
  *channel = info.channel;
  *byte_per_channel = info.byte_per_channel;
  *format = info.format;
  *type = info.type;
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
    CGCHECK(false) << "Unsupported texture format : " << file_ext;
  }
}

// failed return 0
void SaveTexture2D(const std::string& path_with_ext, GLuint tex, bool multiple_sample) {
  CGLOG(INFO) << "[texture::SaveTexture2D] path : " << path_with_ext;
  GLuint target = multiple_sample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
  glBindTexture(target, tex);

  int width = -1, height = -1, internal_format = -1;
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &height);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
  
  int channel = -1, byte_per_channel = -1, format = -1, type = -1;
  GetInternalFormatSize(internal_format, &channel, &byte_per_channel, &format, &type);
  GLubyte* pixels = new GLubyte[width * height * channel * byte_per_channel];

  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
  glGetTexImage(target, 0, format, type, pixels);

  CGCHECK(width > 0) << "Widget must > 0";
  CGCHECK(height > 0) << "Height must > 0";

  CGCHECK(VarifyChannel(path_with_ext, channel)); 
	CGCHECK(SOIL_save_image(path_with_ext.c_str(), SOIL_SAVE_TYPE_PNG, width, height, channel, pixels));

  delete[] pixels;
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
      CGCHECK(false) << util::Format("cannot load image {}", path[i]);
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

    int channel = -1, byte_per_channel = -1, format = -1, type = -1;
    GetInternalFormatSize(internal_format, &channel, &byte_per_channel, &format, &type);
    GLubyte* pixels = new GLubyte[width * height * channel * byte_per_channel];

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

  int channel = -1, byte_per_channel = -1, format = -1, type = -1;
  GetInternalFormatSize(internal_format, &channel, &byte_per_channel, &format, &type);
  GLubyte* pixels = new GLubyte[width * height * channel * byte_per_channel];
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