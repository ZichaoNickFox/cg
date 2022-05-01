#include "playground/texture_repo.h"

#include <glog/logging.h>
#include "SOIL2/SOIL2.h"
#include "SOIL2/stb_image.h"

#include "engine/debug.h"
#include "playground/util.h"

using engine::Texture;

namespace {
void ParseImageFormat(const std::string& file_name, int* SOILfmt, GLint* internal_format) {
  // GL Format
  // https://community.khronos.org/t/gl-rgb-gl-rgba-gl-rgb8-what-is-what/41879
  std::string file_ext = util::GetFileExt(file_name);
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
}

namespace texture {
Texture LoadTexture2D(const std::string& path_with_ext, bool flip_vertically, int mipmap_level_count,
                      bool equirectangular) {
  // OpenGL uv (0,0) is at left bottom
  // Texture uv (0,0) is at left top
  // So flip vertical uv
  stbi_set_flip_vertically_on_load(flip_vertically);

  GLuint id;
  glGenTextures(1, &id);

  int SOILfmt;
  GLint internal_format;
  ParseImageFormat(path_with_ext, &SOILfmt, &internal_format);

  if (equirectangular) {
    int width, height, nr_components;
    float* image = stbi_loadf(path_with_ext.c_str(), &width, &height, &nr_components, 0);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, internal_format, GL_FLOAT, image);
    free(image);
  } else {
    unsigned char* image = nullptr;
    int width, height, nr_components;
    image = SOIL_load_image(path_with_ext.c_str(), &width, &height, 0, SOILfmt);
    if (!image) {
      CGCHECK(false) << util::Format("cannot load image {}", path_with_ext);
    }
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if(mipmap_level_count > 1){
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_MIPMAP);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  return Texture(id, Texture::Texture2D, util::FileName(path_with_ext));
}
}

namespace {
void FlipVertically(GLubyte* pixels, int width, int height, int channel, int byte_per_channel) {
  int byte_per_pixel = channel * byte_per_channel;
  int byte_per_line = width * byte_per_pixel;
  int h1 = height - 1;
  int h2 = 0;
  while (h2 < h1) {
    int h1_first_type = h1 * byte_per_line;
    int h2_first_type = h2 * byte_per_line;
    for (int byte_in_line = 0; byte_in_line < byte_per_line; ++ byte_in_line) {
      std::swap(pixels[h1_first_type + byte_in_line], pixels[h2_first_type + byte_in_line]);
    }
    h2++;
    h1--;
  }
}

void TryMakeDir(const std::string& path_with_ext) {
  std::string file_dir = util::FileDir(path_with_ext);
  util::MakeDir(file_dir);
}

Texture CreateTexture2D(const texture::CreateTexture2DParam& param) {
  GLuint ret;
  glGenTextures(1, &ret);
  glBindTexture(GL_TEXTURE_2D, ret);
  glTexStorage2D(GL_TEXTURE_2D, param.levels, param.internal_format, param.width, param.height);
  for (int level = 0; level < param.levels; ++level) {
    glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, param.width >> level, param.height >> level,
                    param.format, param.type, param.datas[level]);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if(param.levels > 1){
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  CGCHECKGL();
  return Texture(ret, Texture::Texture2D);
}

Texture CreateCubemap(const texture::CreateCubemapParam& param) {
  GLuint ret;
  glGenTextures(1, &ret);
  glBindTexture(GL_TEXTURE_CUBE_MAP, ret);
  glTexStorage2D(GL_TEXTURE_CUBE_MAP, param.levels, param.internal_format, param.width, param.height);
  for (GLuint texture_unit = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
       texture_unit <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; ++texture_unit) {
    for (int level = 0; level < param.levels; ++level) {
      glTexSubImage2D(texture_unit, level, 0, 0, param.width >> level, param.height >> level, param.format,
                      param.type, param.datas[texture_unit - GL_TEXTURE_CUBE_MAP_POSITIVE_X][level]);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if(param.levels > 1){
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  return Texture(ret, Texture::Cubemap);
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

// failed return 0
void SaveTexture2D(const std::string& path_with_ext, const engine::Texture& texture, bool multiple_sample = false) {
  CGLOG(INFO) << "[texture::SaveTexture2D] path : " << path_with_ext;
  GLuint target = multiple_sample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
  glBindTexture(target, texture.id());

  int width = -1, height = -1, internal_format = -1;
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &height);
  glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
  
  int channel = -1, byte_per_channel = -1, format = -1, type = -1;
  GetInternalFormatSize(internal_format, &channel, &byte_per_channel, &format, &type);
  GLubyte pixels[width * height * channel * byte_per_channel];

  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
  glGetTexImage(target, 0, format, type, pixels);
  FlipVertically(pixels, width, height, channel, byte_per_channel);

  CGCHECK(width > 0) << "Widget must > 0";
  CGCHECK(height > 0) << "Height must > 0";

  CGCHECK(VarifyChannel(path_with_ext, channel)); 

  TryMakeDir(path_with_ext);
	CGCHECK(SOIL_save_image(path_with_ext.c_str(), SOIL_SAVE_TYPE_PNG, width, height, channel, pixels));
}

Texture LoadCubeMap(const std::vector<std::string>& path, int mipmap_level_count) {
  GLuint textureId;
  glGenTextures(1, &textureId);
  
  Texture ret(textureId, Texture::Cubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
  for(GLuint i = 0; i < 6; ++i) {
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

/*
int SaveCubemap(const std::vector<std::string>& path_with_exts, GLuint texture) {
  void SaveCubemap(const std::string& name, const std::vector<engine::Texture>& cubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

  for(int i = 0; i < 6; ++i){
    int width = -1, height = -1, internal_format = -1;
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_HEIGHT, &height);
    glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

    int channel = -1, byte_per_channel = -1, format = -1, type = -1;
    GetInternalFormatSize(internal_format, &channel, &byte_per_channel, &format, &type);
    GLubyte pixels[width * height * channel * byte_per_channel];

    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
    glGetTexImage(GL_TEXTURE_2D, 0, format, type, pixels);
    FlipVertically(pixels, width, height, channel, byte_per_channel);

    TryMakeDir(path_with_exts[i]);
    SaveTexture2D(path_with_exts[i], width, height, channel, pixels);
  }
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  return true;
}
*/

Texture Copy(GLuint source) {
  glBindTexture(GL_TEXTURE_2D, source);
  int width = -1, height = -1, internal_format = -1;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

  int channel = -1, byte_per_channel = -1, format = -1, type = -1;
  GetInternalFormatSize(internal_format, &channel, &byte_per_channel, &format, &type);
  GLubyte pixels[width * height * channel * byte_per_channel];
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
} // namespace

void TextureRepo::Init(const Config& config) {
  for (const TextureConfig& config : config.texture_config()) {
    State state;
    state.paths = util::ProtoRepeatedToVector(config.paths());
    state.texture_type = (Texture::Type)config.texture_type();
    textures_[config.name()] = state;
    CGLOG(ERROR) << "Init texture : " << config.name() << " (" << TextureType_Name(config.texture_type()) << ")";
  }
}

Texture TextureRepo::GetOrLoadTexture(const std::string& name, bool flip_vertically, int mipmap_level_count) {
  CGCHECK(textures_.count(name) > 0) << "Cannot find texture : " << name;
  State* state = &textures_[name];
  if (!state->loaded) {
    if (state->texture_type == Texture::Texture2D) {
      CGCHECK(state->paths.size() == 1) << "Texture2D has 1 texture : " << name;
      state->texture = texture::LoadTexture2D(state->paths[0], flip_vertically, mipmap_level_count);
    } else if (state->texture_type == Texture::Cubemap) {
      state->texture = LoadCubeMap(state->paths, mipmap_level_count);
    } else {
      CGCHECK(false) << "Unsupported Texture Type";
    }
    state->loaded = true;
    CGLOG(ERROR) << "Loaded Texture : id - " << state->texture.id() << " | name - " << name;
  }
  return state->texture;
}

void TextureRepo::SaveTexture(const std::string& name, const engine::Texture& texture) {
  State* state = &textures_[name];
  const std::string& path = state->paths[0];
  SaveTexture2D(path, texture);
}

void TextureRepo::SaveCubemap(const std::string& name, int face, const engine::Texture& cubemap_face_texture2d) {
  State* state = &textures_[name];
  CGCHECK(state->paths.size() == 6);
  CGCHECK(face < 6 && face >= 0);
  const std::string& path = state->paths[face];
  SaveTexture2D(path, cubemap_face_texture2d);
}

void TextureRepo::CreateTexture2D(const std::string& name, const texture::CreateTexture2DParam& param) {
  CGCHECK(textures_.count(name) == 0) << "Exist same name texture : " << name;
  State* state = &textures_[name];
  state->texture = ::CreateTexture2D(param);
  state->loaded = true;
  state->texture_type = engine::Texture::Texture2D;
}

void TextureRepo::CreateCubemap(const std::string& name, const texture::CreateCubemapParam& param) {
  CGCHECK(textures_.count(name) == 0) << "Exist same name texture : " << name;
  State* state = &textures_[name];
  state->texture = ::CreateCubemap(param);
  state->loaded = true;
  state->texture_type = engine::Texture::Texture2D;
}