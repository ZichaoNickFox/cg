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

template<typename ChannelType>
void FlipVertically(const Texture::Meta meta, ChannelType* pixels) {
  int pixel_size_in_byte = meta.channel_num * meta.hdr ? 4 : 1;
  int size_per_line = meta.width * pixel_size_in_byte;
  int h1 = meta.height - 1;
  int h2 = 0;
  while (h2 < h1) {
    int h1_first_type = h1 * size_per_line;
    int h2_first_type = h2 * size_per_line;
    for (int byte_in_line = 0; byte_in_line < size_per_line; ++ byte_in_line) {
      std::swap(pixels[h1_first_type + byte_in_line], pixels[h2_first_type + byte_in_line]);
    }
    h2++;
    h1--;
  }
}

void DeleteTexture(GLubyte* texture) {
  CGCHECK(texture);
  STBI_FREE(texture);
} 

void* ReadTextureData(const std::string& filename, const Texture::Meta& meta) {
	void* res = nullptr;
  int width, height, _;
  if (meta.hdr) {
    res = stbi_loadf(filename.c_str(), &width, &height, &_, 4);
  } else {
    res = stbi_load(filename.c_str(), &width, &height, &_, 4);
  }
  CGCHECK(width == meta.width);
  CGCHECK(height == meta.height);
  CGCHECK(res) << stbi_failure_reason() << " " << util::ReplaceBackslash(filename);
	return res;
}

Texture::Meta ReadTextureMeta(const std::string& path) {
  Texture::Meta res;
  int width = -1, height = -1, _ = 4, hdr = -1;
  stbi_info(path.c_str(), &width, &height, &_);
  hdr = stbi_is_hdr(path.c_str());
  CGCHECK(width > 0 && height > 0 && hdr != -1)
      << "width~" << width << " height~" << height << " channel~" << 4 << " hdr~" << hdr;
  res.type = Texture::kTexture2D;
  res.width = width;
  res.height = height;
  res.channel_num = 4;
  res.hdr = (hdr == 1);
  res.level_num = 1;
  res.depth = 1;
  if (hdr == 0) {
    res.gl_internal_format = GL_RGBA8;
    res.gl_format = GL_RGBA;
    res.gl_type = GL_UNSIGNED_BYTE;
  } else {
    res.gl_internal_format = GL_RGBA32F;
    res.gl_format = GL_RGBA32F;
    res.gl_type = GL_FLOAT;
  }
  res.gl_min_filter = GL_LINEAR;
  res.gl_mag_filter = GL_LINEAR;
  res.gl_wrap_s = GL_REPEAT;
  res.gl_wrap_t = GL_REPEAT;
  return res;
}

std::vector<unsigned char> ResizeTexture(const std::vector<unsigned char>& data, const Texture::Meta& meta,
                                         int out_width, int out_height) {
  std::vector<unsigned char> res(meta.resized_data_size_in_byte(out_width, out_height));
  CGCHECK(stbir_resize_uint8(data.data(), meta.width, meta.height, 0, res.data(),
                              out_width, out_height, 0, meta.channel_num));
  return res;
}

std::vector<float> ResizeTextureHDR(const std::vector<float>& data, const Texture::Meta& meta,
                                    int out_width, int out_height) {
  std::vector<float> res(meta.resized_data_size_in_byte(out_width, out_height));
  CGCHECK(stbir_resize_float(data.data(), meta.width, meta.height, 0, res.data(),
                              out_width, out_height, 0, meta.channel_num));
  return res;
}

void MergeTexture(const std::vector<unsigned char>& texture, std::vector<unsigned char>* merged) {
  std::copy(texture.begin(), texture.end(), std::back_inserter(*merged));
}

Texture CreateTexture2D(const Texture::Meta& meta, const std::vector<void*>& datas) {
  meta.Varify();
  Texture res(meta);
  glGenTextures_(1, res.mutable_id());
  glBindTexture_(GL_TEXTURE_2D, res.id());
  glTexStorage2D_(GL_TEXTURE_2D, meta.level_num, meta.gl_internal_format, meta.width, meta.height);
  for (int level = 0; level < meta.level_num; ++level) {
    glTexSubImage2D_(GL_TEXTURE_2D, level, 0, 0, meta.width >> level, meta.height >> level,
                     meta.gl_format, meta.gl_type, datas[level]);
  }
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, meta.gl_wrap_s);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, meta.gl_wrap_t);
  if (meta.level_num > 1) {
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, meta.gl_mag_filter);
  } else {
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, meta.gl_min_filter);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, meta.gl_mag_filter);
  }
  glBindTexture_(GL_TEXTURE_2D, 0);
  res.Varify();
  return res;
}

Texture CreateTexture2D(int width, int height, const std::vector<glm::vec4>& data,
                        GLuint min_filter, GLuint mag_filter, GLuint wrap_s, GLuint wrap_t) {
  CGCHECK(width * height * 4 * 4 == util::VectorSizeInByte(data))
      << "width~" << width << " height~" << height << " data_size~" << data.size();
  Texture::Meta meta;
  meta.type = Texture::kTexture2D;
  meta.width = width;
  meta.height = height;
  meta.channel_num = 4;
  meta.hdr = true;
  meta.level_num = 1;
  meta.depth = 1;

  meta.gl_internal_format = GL_RGBA32F;
  meta.gl_format = GL_RGBA;
  meta.gl_type = GL_FLOAT;
  meta.gl_min_filter = min_filter;
  meta.gl_mag_filter = mag_filter;
  meta.gl_wrap_s = wrap_s;
  meta.gl_wrap_t = wrap_t;
  CGCHECK(meta.data_size_in_byte() == util::VectorSizeInByte(data))
      << meta.data_size_in_byte() << " " << util::VectorSizeInByte(data);
  Texture res = CreateTexture2D(meta, std::vector<void*>{(void*)data.data()});
  res.Varify();
  return res;
}

Texture CreateTexture2DArray(const std::vector<Texture>& textures, int width, int height) {
  std::vector<unsigned char> texture_2d_array;
  for (const Texture& source : textures) {
    CGCHECK(!source.meta().hdr);
    std::vector<unsigned char> data = source.GetData<unsigned char>();
    if (source.meta().width != width || source.meta().height != height) {
      data = ResizeTexture(data, source.meta(), width, height);
    }
    MergeTexture(data, &texture_2d_array);
  }
  Texture::Meta meta;
  meta.type = Texture::kTexture2DArray;
  meta.width = width;
  meta.height = height;
  meta.channel_num = 4;
  meta.hdr = false;
  meta.level_num = 1;
  meta.depth = textures.size();
  meta.gl_internal_format = GL_RGBA8;
  meta.gl_format = GL_RGBA;
  meta.gl_type = GL_UNSIGNED_BYTE;
  meta.gl_min_filter = GL_LINEAR;
  meta.gl_mag_filter = GL_LINEAR;
  meta.gl_wrap_s = GL_REPEAT;
  meta.gl_wrap_t = GL_REPEAT;
  CGCHECK(meta.data_size_in_byte() == texture_2d_array.size())
      << meta.data_size_in_byte() << " " << texture_2d_array.size();
  Texture res(meta);
  glGenTextures_(1, res.mutable_id());
  glBindTexture_(GL_TEXTURE_2D_ARRAY, res.id());
  glTexImage3D_(GL_TEXTURE_2D_ARRAY, 0, meta.gl_internal_format, meta.width, meta.height,
                meta.depth, 0, meta.gl_format, meta.gl_type, texture_2d_array.data());
  glTexParameteri_(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri_(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture_(GL_TEXTURE_2D_ARRAY, 0);
  res.Varify();
  return res;
}

Texture ReadTexture(const std::string& fullpath) {
  Texture::Meta meta;
  meta = ReadTextureMeta(fullpath);
  void* data = ReadTextureData(fullpath, meta);
  Texture res = CreateTexture2D(meta, std::vector<void*>{data});
  res.Varify();
  return res;
}
// Texture2DData::Texture2DData(int level_num, int level0_buffer_size) {
//   resize(level_num, level0_buffer_size);
// }

// void Texture2DData::resize(int level_num, int level0_buffer_size) {
//   data_.resize(level_num);
//   for (int level = 0; level < level_num; ++level) {
//     data_[level].resize(level0_buffer_size / std::pow(4, level));
//   }
// }

// CubemapData::CubemapData(int level_num, int level0_buffer_size) {
//   for (int face = 0; face < 6; ++face) {
//     data_[face].resize(level_num, level0_buffer_size);
//   }
// }

const GLuint& Texture::id() const {
  CGCHECK(id_ != std::numeric_limits<GLuint>::max()) << "Texture used before assign id";
  return id_;
}

bool Texture::Varify() const {
  bool not_empty = !empty();
  CGCHECK(not_empty);
  return not_empty && meta_.Varify();
}

// void SaveImage(const std::string& filename, const Texture::Meta& meta, const Texture2DData& data) {
//   std::string file_ext = util::FileExt(filename);
//   if (file_ext == "bmp") {
//     CGCHECK(stbi_write_bmp(filename.c_str(), width, height, channels, (void*)data));
//   } else if (file_ext == "tga") {
//     CGCHECK(stbi_write_tga(filename.c_str(), width, height, channels, (void*)data));
//   } else if (file_ext == "dds") {
//     CGCHECK(stbi_write_hdr(filename.c_str(), width, height, channels, (float*)data));
//   } else if (file_ext == "png") {
//     CGCHECK(stbi_write_png(filename.c_str(), width, height, channels, (void*)data, 0));
//   } else if (file_ext == "jpg") {
//     CGCHECK(stbi_write_jpg(filename.c_str(), width, height, channels, (void*)data, width * channels));
//   }
// }

// std::string GetCubemapPath(const std::unordered_map<std::string, std::string>& paths,
//                            int level, int texture_unit_offset) {
//   std::vector<std::string> faces = {"px", "nx", "py", "ny", "pz", "nz"};
//   std::string key = fmt::format("level{}_{}", level, faces[texture_unit_offset]);
//   CGCHECK(!paths.empty());
//   CGCHECK(paths.count(key) > 0) << " Cannot find path " << key;
//   return paths.at(key);
// }

void TryMakeDir(const std::string& path_with_ext) {
  std::string file_dir = util::FileDir(path_with_ext);
  util::MakeDir(file_dir);
}

// Texture ResetCubemapImpl(const CubemapParam& param) {
//   GLuint ret;
//   glGenTextures_(1, &ret);
//   glBindTexture_(GL_TEXTURE_CUBE_MAP, ret);
//   glTexStorage2D_(GL_TEXTURE_CUBE_MAP, param.level_num, param.internal_format, param.width, param.height);
//   for (GLuint texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset) {
//     for (int level = 0; level < param.level_num; ++level) {
//       glTexSubImage2D_(GL_TEXTURE_CUBE_MAP_POSITIVE_X + texture_unit_offset, level, 0, 0,
//                        param.width >> level, param.height >> level, param.format,
//                        param.type, param.data->mutable_data(texture_unit_offset, level));
//     }
//   }
//   glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
//   glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
//   if (param.level_num > 1) {
//     glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//   } else {
//     glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   }
//   glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//   glBindTexture_(GL_TEXTURE_CUBE_MAP, 0);
//   return Texture(ret, Texture::Cubemap);
// }

// Outsize cubemap :
//    py
// nx pz px nz
//    ny
// renderer::Texture CreateCubemapPreviewTexture2DImpl(const CubemapParam& param) {
//   GLuint ret;
//   glGenTextures_(1, &ret);
//   glBindTexture_(GL_TEXTURE_2D, ret);
//   glTexStorage2D_(GL_TEXTURE_2D, 1, param.internal_format, 800, 600);
//   // px, nx, py, ny, pz, nz
//   std::vector<glm::vec2> offset{{400, 200}, {0, 200}, {200, 400}, {200, 0}, {200, 200}, {600, 200}};
//   for (GLuint texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset) {
//     glTexSubImage2D_(GL_TEXTURE_2D, 0, offset[texture_unit_offset].x, offset[texture_unit_offset].y,
//                      200, 200, param.format, param.type, param.data->mutable_data(texture_unit_offset, 0));
//   }
//   glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//   glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//   if (param.level_num > 1) {
//     glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//   } else {
//     glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   }
//   glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//   glBindTexture_(GL_TEXTURE_2D, 0);
//   return Texture(ret, Texture::kTexture2D);
// }

bool Texture::Meta::Varify() const {
  CGCHECK(width > 0) << width;
  CGCHECK(height > 0) << height;
  CGCHECK(channel_num == 4) << channel_num;
  CGCHECK(type != Texture::kUnknown) << type;
  CGCHECK(level_num != -1) << level_num;
  CGCHECK(depth != -1) << depth;
  CGCHECK(gl_internal_format != -1) << gl_internal_format;
  CGCHECK(gl_format != -1) << gl_format;
  CGCHECK(gl_type != -1) << gl_type;
  CGCHECK(gl_min_filter != -1) << gl_min_filter;
  CGCHECK(gl_mag_filter != -1) << gl_mag_filter;
  CGCHECK(gl_wrap_s != -1) << gl_wrap_s;
  CGCHECK(gl_wrap_t != -1) << gl_wrap_t;
  return true;
}

int Texture::Meta::data_size_in_byte(int level) const {
  int channel_size_in_byte = hdr ? 4 : 1;   // GLfloat GLubyte
  return width * height * depth * channel_num * channel_size_in_byte;
}

int Texture::Meta::resized_data_size_in_byte(int resized_width, int resized_height, int level) const {
  int channel_size_in_byte = hdr ? 4 : 1;   // GLfloat GLubyte
  return resized_width * resized_height * depth * channel_num * channel_size_in_byte;
}

// void SaveTexture2DImpl(const std::unordered_map<std::string, std::string>& paths,
//                        const renderer::Texture& texture, int level_num, bool multiple_sample = false) {
//   GLuint target = multiple_sample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
//   glBindTexture_(target, texture.id());
//   int w = texture.width();
//   int h = texture.height();
//   int channel_num = texture.channel_num();
//   int channel_size_in_byte = texture.channel_size_in_byte();
//   int external_format = texture.external_format();
//   int external_type = texture.external_type();
//   for (int level = 0; level < level_num; ++level) {
//     std::string path = GetTexture2DPath(paths, level);
//     std::vector<GLubyte> pixels(w * h * channel_num * channel_size_in_byte);
//     glGetTexImage_(target, level, external_format, external_type, pixels.data());
//     FlipVertically(pixels.data(), w, h, channel_num, channel_size_in_byte);
//     CGCHECK(w > 0) << "Widget must > 0";
//     CGCHECK(h > 0) << "Height must > 0";
//     CGCHECK(VarifyChannel(path, channel_num)); 
//     TryMakeDir(path);
//     SaveImage(path, w, h, channel_num, pixels.data());
//   }
// }

// Texture CreateCubeMapImpl(const std::vector<std::string>& paths, const ) {
//   stbi_set_flip_vertically_on_load(flip_vertically);

//   GLuint textureId;
//   glGenTextures_(1, &textureId);
  
//   Texture res(textureId, Texture::kCubemap);
//   glBindTexture_(GL_TEXTURE_CUBE_MAP, textureId);

//   std::string key = "level0_pz";
//   CGCHECK(paths.count(key) > 0) << " Cannot find key " << key;
//   std::string path = paths.at(key);
//   ReadTextureMeta(path);

//   glTexStorage2D_(GL_TEXTURE_CUBE_MAP, level_num, GL_RGBA8, base_width, base_height);
//   for(int texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset) {
//     for (int level = 0; level < level_num; ++level) {
//       std::string cubemap_path = GetCubemapPath(paths, level, texture_unit_offset);
//       int width, height;
//       GLubyte* image = ReadTexture(cubemap_path, &width, &height, 0, 4);
//       CGCHECK(width == (base_width >> level)) << " level " << level <<
//           " size should be " << (base_width >> level) << " rather than " << width;
//       CGCHECK(height == (base_height >> level)) << " level " << level <<
//           " size should be " << (base_height >> level) << " rather than " << height;
//       LOG(ERROR) << cubemap_path;
//       glTexSubImage2D_(GL_TEXTURE_CUBE_MAP_POSITIVE_X + texture_unit_offset, level, 0, 0,
//           width, height, format, type, image);
//       DeleteTexture(image);
//     }
//   }
//   glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//   glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//   glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//   glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

//   glBindTexture_(GL_TEXTURE_CUBE_MAP, 0);

//   return res;
// }

// int SaveCubemapImpl(const std::unordered_map<std::string, std::string>& paths,
//                     const renderer::Texture& texture, int level_num) {
//   glBindTexture_(GL_TEXTURE_CUBE_MAP, texture.id());
//   int w = texture.width();
//   int h = texture.height();
//   int channel_num = texture.channel_num();
//   int channel_size_in_byte = texture.channel_size_in_byte();
//   int external_format = texture.external_format();
//   int external_type = texture.external_type();
//   for(int texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset){
//     for (int level = 0; level < level_num; ++level) {
//       std::vector<GLubyte> pixels(w * h * channel_num * channel_size_in_byte);
//       glGetTexImage_(GL_TEXTURE_CUBE_MAP + texture_unit_offset, level, external_format, external_type, pixels.data());
//       FlipVertically(pixels.data(), w, h, channel_num, channel_size_in_byte);
//       std::string file_path = GetCubemapPath(paths, level, texture_unit_offset);
//       TryMakeDir(file_path);
//       SaveImage(file_path, w, h, channel_num, pixels.data());
//     }
//   }
//   glBindTexture_(GL_TEXTURE_CUBE_MAP, 0);
//   return true;
// }

void TextureRepo::Init(const Config& config) {
  config_ = &config;
}

// void TextureRepo::SaveTexture2D(const std::string& name) {
//   LOG(ERROR) << "TextureRepo::SaveTexture2D " << name;
//   CGCHECK(textures_.count(name) > 0) << name;
//   State* state = &textures_[name];
//   CGCHECK(state->texture.type() == renderer::Texture::Type::kTexture2D);
//   SaveTexture2DImpl(state->paths, state->texture, state->level_num);
// }

// void TextureRepo::SaveCubemap(const std::string& name) {
//   LOG(ERROR) << "TextureRepo::SaveCubemap " << name;
//   CGCHECK(textures_.count(name) > 0) << name;
//   State* state = &textures_[name];
//   CGCHECK(state->texture.type() == renderer::Texture::Type::kCubemap);
//   const std::unordered_map<std::string, std::string>& path = state->paths;
//   SaveCubemapImpl(path, state->texture, state->level_num);
// }

// void TextureRepo::ResetTexture2D(const std::string& name, const TextureParam& param) {
//   CGCHECK(textures_.count(name) > 0) << name;
//   LOG(ERROR) << "TextureRepo::ResetTexture2D " << name;
//   State* state = &textures_[name];
//   state->texture = CreateTextureImpl(param);
//   state->loaded = true;
//   state->texture_type = renderer::Texture::Texture2D;
// }

// Texture TextureRepo::CreateTexture(const TextureParam& param) {
//   LOG(ERROR) << "TextureRepo::CreateTexture ";
//   return CreateTextureImpl(param);
// }

// Texture TextureRepo::CreateCubemapPreviewTexture2D(const CubemapParam& param) {
//   LOG(ERROR) << "TextureRepo::CreateCubemapPreviewTexture2D";
//   return CreateCubemapPreviewTexture2DImpl(param);
// }

// void TextureRepo::ResetCubemap(const std::string& name, const CubemapParam& param) {
//   CGCHECK(textures_.count(name) > 0) << name;
//   LOG(ERROR) << "TextureRepo::ResetCubemap " << name;
//   State* state = &textures_[name];
//   state->texture = ResetCubemapImpl(param);
//   state->loaded = true;
//   state->texture_type = renderer::Texture::Cubemap;
// }

int TextureRepo::AddUnique(const std::string& full_path) {
  if (Has(full_path)) {
    return name_2_index_[full_path];
  }

  int index = index_2_texture_.size();
  name_2_index_[full_path] = index;
  index_2_texture_[index] = ReadTexture(full_path);
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

int TextureRepo::size() const {
  CGCHECK(name_2_index_.size() == index_2_texture_.size());
  return index_2_texture_.size();
}

Texture TextureRepo::AsTextureRepo(int width, int height) const {
  bool dirty = (dirty_index_2_texture_ != index_2_texture_);
  if (dirty) {
    std::vector<Texture> textures;
    for (const auto& p : index_2_texture_) {
      textures.push_back(p.second);
    }
    if (!texture_repo_.empty()) {
      glDeleteTextures(1, &texture_repo_.id());
    }
    texture_repo_ = CreateTexture2DArray(textures, width, height);
    dirty_index_2_texture_ = index_2_texture_;
  }
  texture_repo_.Varify();
  return texture_repo_;
}
} // namespace renderer