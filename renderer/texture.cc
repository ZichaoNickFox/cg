#include "renderer/texture.h"

#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include <unordered_map>

#include "renderer/debug.h"
#include "renderer/util.h"

namespace renderer {
namespace {
int SizeInByte(int width, int height, int channels) {
  return width * height * channels;
}
void DeleteImage(GLubyte* image) {
  CGCHECK(image);
  free(image);
} 
GLubyte* ReadImage(const std::string& filename, int* width, int* height, int* channels, int force_channels = 4) {
	GLubyte* res = stbi_load(util::ReplaceBackslash(filename).c_str(), width, height, channels, force_channels);
  CGCHECK(res) << stbi_failure_reason() << " " << util::ReplaceBackslash(filename);
	return res;
}
std::vector<GLubyte> ReadImageAsVector(const std::string& filename, int* width, int* height, int* channels,
                                       int force_channels = 4) {
	GLubyte* image = stbi_load(util::ReplaceBackslash(filename).c_str(), width, height, channels, force_channels);
  CGCHECK(image) << stbi_failure_reason() << " " << util::ReplaceBackslash(filename);
  std::vector<GLubyte> res(image, image + SizeInByte(*width, *height, *channels));
  DeleteImage(image);
	return res;
}
void ReadImageSize(const std::string& path, int* width, int* height) {
  // TODO : Just for width, height. Find a better method
  DeleteImage(ReadImage(path, width, height, 0, 4));
}
std::vector<unsigned char> ResizeImage(const std::vector<unsigned char>& data, int in_width, int in_height,
                                       int out_width, int out_height, int channels = 4) {
  std::vector<unsigned char> res;
  stbir_resize_uint8(data.data(), in_width, in_height, 0, res.data(), out_width, out_height, 0, channels);
  return res;
}
void MergeImage(const std::vector<unsigned char>& texture, std::vector<unsigned char>* merged) {
  std::copy(texture.begin(), texture.end(), std::back_inserter(*merged));
}
}
// Maybe deprecated
struct InternalFormatMeta {
  int channel;
  int byte_per_channel;
  int format;
  int type;
  std::string log;
};
// GL Format
// https://www.cs.uregina.ca/Links/class-info/315/WWW/Lab5/InternalFormats_OGL_Core_3_2.html
const std::unordered_map<int, InternalFormatMeta> internal_format_map = {
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

Texture2DData::Texture2DData(int level_num, int level0_buffer_size) {
  resize(level_num, level0_buffer_size);
}

void Texture2DData::resize(int level_num, int level0_buffer_size) {
  data_.resize(level_num);
  for (int level = 0; level < level_num; ++level) {
    data_[level].resize(level0_buffer_size / std::pow(4, level));
  }
}

CubemapData::CubemapData(int level_num, int level0_buffer_size) {
  for (int face = 0; face < 6; ++face) {
    data_[face].resize(level_num, level0_buffer_size);
  }
}

TextureParam::TextureParam(int in_width, int in_height, const std::vector<unsigned char>& in_raw_data,
                           GLuint in_min_filter, GLuint in_mag_filter, GLuint in_wrap_s, GLuint in_wrap_t) {
  level_num = 1;
  width = in_width;
  height = in_height;
  raw_data.push_back(in_raw_data.data());
  internal_format = GL_RGBA8;
  format = GL_RGBA;
  type = GL_UNSIGNED_BYTE;
  min_filter = in_min_filter;
  mag_filter = in_mag_filter;
  wrap_s = in_wrap_s;
  wrap_t = in_wrap_t;
}

TextureParam::TextureParam(int in_width, int in_height, const std::vector<glm::vec3>& in_raw_data,
                           GLuint in_min_filter, GLuint in_mag_filter, GLuint in_wrap_s, GLuint in_wrap_t) {
  level_num = 1;
  width = in_width;
  height = in_height;
  raw_data.push_back(in_raw_data.data());
  internal_format = GL_RGB32F;
  format = GL_RGB;
  type = GL_FLOAT;
  min_filter = in_min_filter;
  mag_filter = in_mag_filter;
  wrap_s = in_wrap_s;
  wrap_t = in_wrap_t;
}

TextureParam::TextureParam(int in_width, int in_height, const std::vector<glm::vec4>& in_raw_data,
                           GLuint in_min_filter, GLuint in_mag_filter, GLuint in_wrap_s, GLuint in_wrap_t) {
  level_num = 1;
  width = in_width;
  height = in_height;
  raw_data.push_back(in_raw_data.data());
  internal_format = GL_RGBA32F;
  format = GL_RGBA;
  type = GL_FLOAT;
  min_filter = in_min_filter;
  mag_filter = in_mag_filter;
  wrap_s = in_wrap_s;
  wrap_t = in_wrap_t;
}

const void* TextureParam::texture_data(int level) const {
  if (raw_data.size() > 0) {
    CGCHECK(level < raw_data.size()) << " level must LT data.size : level -" << level
                                     << " data.size - " << raw_data.size();
    return raw_data[level];
  } else {
    CGCHECK(level < full_data->level_num()) << " level must LT data.size : level -" << level
                                            << " data.size - " << full_data->level_num();
    return full_data->data(level);
  }
}

const GLuint& Texture::id() const {
  CGCHECK(id_ != std::numeric_limits<GLuint>::max()) << "Texture used before assign id";
  return id_;
}

int Texture::width() const {
  int width;
  glGetTexLevelParameteriv_(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  return width;
}

int Texture::height() const {
  int height;
  glGetTexLevelParameteriv_(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
  return height;
}

int Texture::internal_format() const {
  int res;
  glGetTexLevelParameteriv_(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &res);
  return res;
}

int Texture::channel_size_in_byte() const {
  return internal_format_map.at(internal_format()).byte_per_channel;
}

std::vector<unsigned char> Texture::GetData(int level, bool multiple_sample) const {
  GLuint target = multiple_sample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
  glBindTexture_(target, id_);
  int w = width();
  int h = height();
  int channel_num = this->channel_num();
  int channel_size_in_byte = this->channel_size_in_byte();
  int external_format = this->external_format();
  int external_type = this->external_type();
  std::vector<GLubyte> pixels(w * h * channel_num * channel_size_in_byte);
  glGetTexImage_(target, level, external_format, external_type, pixels.data());
  FlipVertically(pixels.data(), w, h, channel_num, channel_size_in_byte);
  CGCHECK(w > 0) << "Widget must > 0";
  CGCHECK(h > 0) << "Height must > 0";
  return pixels;
}

int Texture::channel_num() const {
  return internal_format_map.at(internal_format()).channel;
}

int Texture::external_format() const {
  return internal_format_map.at(internal_format()).format;
}

int Texture::external_type() const {
  return internal_format_map.at(internal_format()).type;
}

void SaveImage(const std::string& filename, int width, int height, int channels, GLubyte* data) {
  std::string file_ext = util::FileExt(filename);
  if (file_ext == "bmp") {
    CGCHECK(stbi_write_bmp(filename.c_str(), width, height, channels, (void*)data));
  } else if (file_ext == "tga") {
    CGCHECK(stbi_write_tga(filename.c_str(), width, height, channels, (void*)data));
  } else if (file_ext == "dds") {
    CGCHECK(stbi_write_hdr(filename.c_str(), width, height, channels, (float*)data));
  } else if (file_ext == "png") {
    CGCHECK(stbi_write_png(filename.c_str(), width, height, channels, (void*)data, 0));
  } else if (file_ext == "jpg") {
    CGCHECK(stbi_write_jpg(filename.c_str(), width, height, channels, (void*)data, width * channels));
  }
}

std::string GetCubemapPath(const std::unordered_map<std::string, std::string>& paths,
                           int level, int texture_unit_offset) {
  std::vector<std::string> faces = {"px", "nx", "py", "ny", "pz", "nz"};
  std::string key = util::Format("level{}_{}", level, faces[texture_unit_offset]);
  CGCHECK(!paths.empty());
  CGCHECK(paths.count(key) > 0) << " Cannot find path " << key;
  return paths.at(key);
}

std::string GetTexture2DPath(const std::unordered_map<std::string, std::string>& paths, int level) {
  std::string key = util::Format("level{}", level);
  CGCHECK(!paths.empty());
  CGCHECK(paths.count(key) > 0) << " Cannot find path " << key;
  return paths.at(key);
}

Texture LoadTexture2D(const std::unordered_map<std::string, std::string>& paths,
                      bool flip_vertically, int level_num, bool equirectangular) {
  // OpenGL uv (0,0) is at left bottom
  // Texture uv (0,0) is at left top
  // So flip vertical uv
  stbi_set_flip_vertically_on_load(flip_vertically);

  GLuint id;
  glGenTextures_(1, &id);
  glBindTexture_(GL_TEXTURE_2D, id);
  if (equirectangular) {
    int width, height, nr_channels;
    // For width, height
    CGCHECK(stbi_loadf(GetTexture2DPath(paths, 0).c_str(), &width, &height, &nr_channels, 0));
    glTexStorage2D_(GL_TEXTURE_2D, level_num, GL_RGBA16F, width, height);
    for (int level = 0; level < level_num; ++level) {
      float* image = stbi_loadf(GetTexture2DPath(paths, level).c_str(), &width, &height, &nr_channels, 0);
      CGCHECK(image);
      glTexSubImage2D_(GL_TEXTURE_2D, level, 0, 0, width, height, GL_RGBA, GL_FLOAT, image);
      free(image);
    }
  } else {
    int width, height;
    std::string path = GetTexture2DPath(paths, 0);
    ReadImageSize(path, &width, &height);
    glTexStorage2D_(GL_TEXTURE_2D, level_num, GL_RGBA8, width, height);
    for (int level = 0; level < level_num; ++level) {
      GLubyte* image = ReadImage(GetTexture2DPath(paths, level), &width, &height, 0, 4);
      glTexSubImage2D_(GL_TEXTURE_2D, level, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
      DeleteImage(image);
    }
  }
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if(level_num > 1){
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_MIPMAP);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_MIPMAP);
  } else {
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  glBindTexture_(GL_TEXTURE_2D, 0);
  return Texture(id, Texture::Texture2D);
}

void TryMakeDir(const std::string& path_with_ext) {
  std::string file_dir = util::FileDir(path_with_ext);
  util::MakeDir(file_dir);
}

Texture CreateTextureImpl(const TextureParam& param) {
  GLuint ret;
  glGenTextures_(1, &ret);
  glBindTexture_(GL_TEXTURE_2D, ret);
  glTexStorage2D_(GL_TEXTURE_2D, param.level_num, param.internal_format, param.width, param.height);
  for (int level = 0; level < param.level_num; ++level) {
    glTexSubImage2D_(GL_TEXTURE_2D, level, 0, 0, param.width >> level, param.height >> level,
                     param.format, param.type, param.texture_data(level));
  }
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param.wrap_s);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param.wrap_t);
  if(param.level_num > 1){
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param.min_filter);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param.mag_filter);
  }
  glBindTexture_(GL_TEXTURE_2D, 0);
  renderer::Texture res(ret, Texture::Texture2D);
  return res;
}

Texture ResetCubemapImpl(const CubemapParam& param) {
  GLuint ret;
  glGenTextures_(1, &ret);
  glBindTexture_(GL_TEXTURE_CUBE_MAP, ret);
  glTexStorage2D_(GL_TEXTURE_CUBE_MAP, param.level_num, param.internal_format, param.width, param.height);
  for (GLuint texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset) {
    for (int level = 0; level < param.level_num; ++level) {
      glTexSubImage2D_(GL_TEXTURE_CUBE_MAP_POSITIVE_X + texture_unit_offset, level, 0, 0,
                       param.width >> level, param.height >> level, param.format,
                       param.type, param.data->mutable_data(texture_unit_offset, level));
    }
  }
  glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (param.level_num > 1) {
    glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  } else {
    glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture_(GL_TEXTURE_CUBE_MAP, 0);
  return Texture(ret, Texture::Cubemap);
}

// Outsize cubemap :
//    py
// nx pz px nz
//    ny
renderer::Texture CreateCubemapPreviewTexture2DImpl(const CubemapParam& param) {
  GLuint ret;
  glGenTextures_(1, &ret);
  glBindTexture_(GL_TEXTURE_2D, ret);
  glTexStorage2D_(GL_TEXTURE_2D, 1, param.internal_format, 800, 600);
  // px, nx, py, ny, pz, nz
  std::vector<glm::vec2> offset{{400, 200}, {0, 200}, {200, 400}, {200, 0}, {200, 200}, {600, 200}};
  for (GLuint texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset) {
    glTexSubImage2D_(GL_TEXTURE_2D, 0, offset[texture_unit_offset].x, offset[texture_unit_offset].y,
                     200, 200, param.format, param.type, param.data->mutable_data(texture_unit_offset, 0));
  }
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (param.level_num > 1) {
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  } else {
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture_(GL_TEXTURE_2D, 0);
  return Texture(ret, Texture::Texture2D);
}

bool VarifyChannel(const std::string& path, int channel) {
  std::string file_ext = util::FileExt(path);
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

void SaveTexture2DImpl(const std::unordered_map<std::string, std::string>& paths,
                       const renderer::Texture& texture, int level_num, bool multiple_sample = false) {
  GLuint target = multiple_sample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
  glBindTexture_(target, texture.id());
  int w = texture.width();
  int h = texture.height();
  int channel_num = texture.channel_num();
  int channel_size_in_byte = texture.channel_size_in_byte();
  int external_format = texture.external_format();
  int external_type = texture.external_type();
  for (int level = 0; level < level_num; ++level) {
    std::string path = GetTexture2DPath(paths, level);
    std::vector<GLubyte> pixels(w * h * channel_num * channel_size_in_byte);
    glGetTexImage_(target, level, external_format, external_type, pixels.data());
    FlipVertically(pixels.data(), w, h, channel_num, channel_size_in_byte);
    CGCHECK(w > 0) << "Widget must > 0";
    CGCHECK(h > 0) << "Height must > 0";
    CGCHECK(VarifyChannel(path, channel_num)); 
    TryMakeDir(path);
    SaveImage(path, w, h, channel_num, pixels.data());
  }
}

Texture LoadCubeMap(const std::unordered_map<std::string, std::string>& paths, int level_num,
                    bool flip_vertically = false) {
  // OpenGL uv (0,0) is at left bottom
  // Texture uv (0,0) is at left top
  // So flip vertical uv
  stbi_set_flip_vertically_on_load(flip_vertically);

  GLuint textureId;
  glGenTextures_(1, &textureId);
  
  Texture ret(textureId, Texture::Cubemap);
  glBindTexture_(GL_TEXTURE_CUBE_MAP, textureId);

  std::string key = "level0_pz";
  CGCHECK(paths.count(key) > 0) << " Cannot find key " << key;
  std::string path = paths.at(key);
  // TODO : To be variables
  GLint format = GL_RGBA, type = GL_UNSIGNED_BYTE;

  // TODO : This Read / Delete for width, height. find a better method
  int base_width, base_height;
  ReadImageSize(path, &base_width, &base_height);

  glTexStorage2D_(GL_TEXTURE_CUBE_MAP, level_num, GL_RGBA8, base_width, base_height);
  for(int texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset) {
    for (int level = 0; level < level_num; ++level) {
      std::string cubemap_path = GetCubemapPath(paths, level, texture_unit_offset);
      int width, height;
      GLubyte* image = ReadImage(cubemap_path, &width, &height, 0, 4);
      CGCHECK(width == (base_width >> level)) << " level " << level <<
          " size should be " << (base_width >> level) << " rather than " << width;
      CGCHECK(height == (base_height >> level)) << " level " << level <<
          " size should be " << (base_height >> level) << " rather than " << height;
      LOG(ERROR) << cubemap_path;
      glTexSubImage2D_(GL_TEXTURE_CUBE_MAP_POSITIVE_X + texture_unit_offset, level, 0, 0,
          width, height, format, type, image);
      DeleteImage(image);
    }
  }
  glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glBindTexture_(GL_TEXTURE_CUBE_MAP, 0);

  return ret;
}

int SaveCubemapImpl(const std::unordered_map<std::string, std::string>& paths,
                    const renderer::Texture& texture, int level_num) {
  glBindTexture_(GL_TEXTURE_CUBE_MAP, texture.id());
  int w = texture.width();
  int h = texture.height();
  int channel_num = texture.channel_num();
  int channel_size_in_byte = texture.channel_size_in_byte();
  int external_format = texture.external_format();
  int external_type = texture.external_type();
  for(int texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset){
    for (int level = 0; level < level_num; ++level) {
      std::vector<GLubyte> pixels(w * h * channel_num * channel_size_in_byte);
      glGetTexImage_(GL_TEXTURE_CUBE_MAP + texture_unit_offset, level, external_format, external_type, pixels.data());
      FlipVertically(pixels.data(), w, h, channel_num, channel_size_in_byte);
      std::string file_path = GetCubemapPath(paths, level, texture_unit_offset);
      TryMakeDir(file_path);
      SaveImage(file_path, w, h, channel_num, pixels.data());
    }
  }
  glBindTexture_(GL_TEXTURE_CUBE_MAP, 0);
  return true;
}

void TextureRepo::Init(const Config& config) {
  for (const auto& p : config.texture_configs()) {
    const TextureConfig& config = p.second;
    State state;
    state.paths = util::ProtoMap2UnorderedMap(config.path());
    state.texture_type = (Texture::Type)config.texture_type();
    state.level_num = config.level_num();
    textures_[config.name()] = state;
    CGLOG(ERROR) << "Init texture : " << config.name() << " (" << TextureType_Name(config.texture_type()) << ")";
  }
}

Texture TextureRepo::GetOrLoadTexture(const std::string& name, bool flip_vertically) {
  CGCHECK(textures_.count(name) > 0) << "Cannot find texture : " << name;
  State* state = &textures_[name];
  if (!state->loaded) {
    if (state->texture_type == Texture::Texture2D) {
      LOG(ERROR) << "Loading texture2D " << name;
      CGCHECK(state->paths.size() == 1) << "Texture2D has 1 texture : " << name;
      state->texture = LoadTexture2D(state->paths, flip_vertically, state->level_num, false);
    } else if (state->texture_type == Texture::Cubemap) {
      LOG(ERROR) << "Loading Cubemap " << name;
      state->texture = LoadCubeMap(state->paths, state->level_num, flip_vertically);
    } else {
      CGCHECK(false) << "Unsupported Texture Type";
    }
    state->loaded = true;
    CGLOG(ERROR) << "Loaded Texture : id - " << state->texture.id() << " | name - " << name;
  }
  return state->texture;
}

void TextureRepo::SaveTexture2D(const std::string& name) {
  LOG(ERROR) << "TextureRepo::SaveTexture2D " << name;
  CGCHECK(textures_.count(name) > 0) << name;
  State* state = &textures_[name];
  CGCHECK(state->texture.type() == renderer::Texture::Type::Texture2D);
  SaveTexture2DImpl(state->paths, state->texture, state->level_num);
}

void TextureRepo::SaveCubemap(const std::string& name) {
  LOG(ERROR) << "TextureRepo::SaveCubemap " << name;
  CGCHECK(textures_.count(name) > 0) << name;
  State* state = &textures_[name];
  CGCHECK(state->texture.type() == renderer::Texture::Type::Cubemap);
  const std::unordered_map<std::string, std::string>& path = state->paths;
  SaveCubemapImpl(path, state->texture, state->level_num);
}

void TextureRepo::ResetTexture2D(const std::string& name, const TextureParam& param) {
  CGCHECK(textures_.count(name) > 0) << name;
  LOG(ERROR) << "TextureRepo::ResetTexture2D " << name;
  State* state = &textures_[name];
  state->texture = CreateTextureImpl(param);
  state->loaded = true;
  state->texture_type = renderer::Texture::Texture2D;
}

Texture TextureRepo::CreateTexture(const TextureParam& param) {
  LOG(ERROR) << "TextureRepo::CreateTexture ";
  return CreateTextureImpl(param);
}

Texture TextureRepo::CreateCubemapPreviewTexture2D(const CubemapParam& param) {
  LOG(ERROR) << "TextureRepo::CreateCubemapPreviewTexture2D";
  return CreateCubemapPreviewTexture2DImpl(param);
}

void TextureRepo::ResetCubemap(const std::string& name, const CubemapParam& param) {
  CGCHECK(textures_.count(name) > 0) << name;
  LOG(ERROR) << "TextureRepo::ResetCubemap " << name;
  State* state = &textures_[name];
  state->texture = ResetCubemapImpl(param);
  state->loaded = true;
  state->texture_type = renderer::Texture::Cubemap;
}

Texture TextureRepo::LoadModelTexture2D(const std::string& full_path, bool flip_vertically) {
  stbi_set_flip_vertically_on_load(flip_vertically);

  GLuint id;
  glGenTextures_(1, &id);
  glBindTexture_(GL_TEXTURE_2D, id);
  int width, height, channel;
  GLubyte* image = ReadImage(full_path, &width, &height, &channel, 4);
  glTexImage2D_(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture_(GL_TEXTURE_2D, 0);

  DeleteImage(image);
  return Texture(id, Texture::Texture2D, full_path);
}

// Formal
int TextureRepo::AddUnique(const std::string& path) {
  if (Has(path)) {
    return name_2_index_[path];
  }
  int index = index_2_texture_.size();
  name_2_index_[path] = index;
  int width, height, channel;
  std::vector<GLubyte> data = ReadImageAsVector(path, &width, &height, &channel);
  TextureParam param(width, height, data);
  index_2_texture_[index] = CreateTexture(param);
  return index;
}

int TextureRepo::AddUnique(const std::string& name, const Texture& texture) {
  if (Has(name)) {
    return name_2_index_[name];
  }
  int index = index_2_texture_.size();
  name_2_index_[name] = index;
  index_2_texture_[index] = texture;
  return index;
}

bool TextureRepo::Has(const std::string& path) const {
  CGCHECK(name_2_index_.size() == index_2_texture_.size());
  return name_2_index_.find(path) != name_2_index_.end();
}

bool TextureRepo::Has(int index) const {
  CGCHECK(name_2_index_.size() == index_2_texture_.size());
  return index_2_texture_.find(index) != index_2_texture_.end();
}

int TextureRepo::TextureNum() const {
  CGCHECK(name_2_index_.size() == index_2_texture_.size());
  return index_2_texture_.size();
}

Texture TextureRepo::AsTextureRepo(int width, int height) const {
  if (dirty_) {
    std::vector<unsigned char> all_data;
    for (int i = 0; i < TextureNum(); ++i) {
      Texture source = index_2_texture_.at(i);
      std::vector<unsigned char> data = ResizeImage(source.GetData(), source.width(), source.height(), width, height);
      MergeImage(data, &all_data);
    }
    if (!texture_repo_.empty()) {
      glDeleteTextures(1, &texture_repo_.id());
    }
    GLuint handle;
    glGenTextures_(1, &handle);
    glBindTexture_(GL_TEXTURE_2D_ARRAY, handle);
    glTexImage3D_(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height,
                  TextureNum(), 0, GL_RGBA, GL_UNSIGNED_BYTE, all_data.data());
    glTexParameteri_(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture_(GL_TEXTURE_2D_ARRAY, 0);

    texture_repo_ = Texture(handle, Texture::Type::Texture3D);
    dirty_ = false;
  }
  return texture_repo_;
}
} // namespace renderer