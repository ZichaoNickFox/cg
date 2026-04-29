#include "renderer/texture.h"

#include <cstring>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#include <unordered_map>

#include "base/debug.h"
#include "base/util.h"
#include "rhi/device.h"

namespace cg {
namespace {

constexpr int kCubemapFaceCount = 6;

std::vector<uint8_t> CopyBytes(const void* data, size_t size_in_bytes) {
  std::vector<uint8_t> bytes(size_in_bytes);
  if (size_in_bytes > 0) {
    CGCHECK(data != nullptr) << "Texture upload payload cannot be null when size is positive.";
    std::memcpy(bytes.data(), data, size_in_bytes);
  }
  return bytes;
}

int TextureFaceCount(const Texture::Meta& meta) {
  return meta.type == Texture::kCubemap ? kCubemapFaceCount : 1;
}

size_t TextureSubresourceIndex(const Texture::Meta& meta, int face, int level) {
  CGCHECK(level >= 0 && level < meta.level_num) << level;
  const int face_count = TextureFaceCount(meta);
  CGCHECK(face >= 0 && face < face_count) << face;
  return static_cast<size_t>(level * face_count + face);
}

}  // namespace

Texture::Texture(const Texture::Meta& meta)
    : storage_(std::make_shared<Storage>()),
      meta_(meta) {}

Texture::Texture(uint32_t id, const Texture::Meta& meta)
    : Texture(id, meta, "") {}

Texture::Texture(uint32_t id, const Texture::Meta& meta, const std::string& info)
    : storage_(std::make_shared<Storage>()),
      meta_(meta),
      info_(info) {
  storage_->id = id;
  storage_->uploaded_to_gl = (id != std::numeric_limits<uint32_t>::max());
  storage_->owns_gl_texture = false;
}

template<typename ChannelType>
void FlipVertically(const Texture::Meta meta, ChannelType* pixels) {
  int pixel_size_in_byte = meta.channel_num * (meta.hdr ? 4 : 1);
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

void DeleteTexture(stbi_uc* texture) {
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
    res.format = rhi::TextureFormat::kRGBA8;
    res.pixel_format = rhi::PixelFormat::kRGBA;
    res.pixel_type = rhi::PixelType::kUInt8;
  } else {
    res.format = rhi::TextureFormat::kRGBA32F;
    res.pixel_format = rhi::PixelFormat::kRGBA;
    res.pixel_type = rhi::PixelType::kFloat32;
  }
  res.min_filter = rhi::FilterMode::kLinear;
  res.mag_filter = rhi::FilterMode::kLinear;
  res.wrap_s = rhi::WrapMode::kRepeat;
  res.wrap_t = rhi::WrapMode::kRepeat;
  return res;
}

std::vector<unsigned char> ResizeTexture(const std::vector<unsigned char>& data, const Texture::Meta& meta,
                                         int out_width, int out_height) {
  std::vector<unsigned char> res(meta.resized_data_size_in_byte(out_width, out_height));
  CGCHECK(stbir_resize_uint8_linear(data.data(), meta.width, meta.height, 0, res.data(),
                                    out_width, out_height, 0,
                                    static_cast<stbir_pixel_layout>(meta.channel_num)));
  return res;
}

std::vector<float> ResizeTextureHDR(const std::vector<float>& data, const Texture::Meta& meta,
                                    int out_width, int out_height) {
  std::vector<float> res(meta.resized_data_size_in_byte(out_width, out_height));
  CGCHECK(stbir_resize_float_linear(data.data(), meta.width, meta.height, 0, res.data(),
                                    out_width, out_height, 0,
                                    static_cast<stbir_pixel_layout>(meta.channel_num)));
  return res;
}

void MergeTexture(const std::vector<unsigned char>& texture, std::vector<unsigned char>* merged) {
  std::copy(texture.begin(), texture.end(), std::back_inserter(*merged));
}

Texture CreateTexture2D(const Texture::Meta& meta, const std::vector<void*>& datas) {
  meta.Varify();
  Texture res(meta);
  for (int level = 0; level < meta.level_num; ++level) {
    res.SetCpuLevelData(level, datas[level], meta.data_size_in_byte(level));
  }
  res.Varify();
  return res;
}

Texture CreateCubemap(const Texture::Meta& meta, const std::vector<void*>& datas) {
  meta.Varify();
  CGCHECK(meta.type == Texture::kCubemap);
  CGCHECK(meta.depth == kCubemapFaceCount) << meta.depth;
  CGCHECK(datas.size() == static_cast<size_t>(meta.level_num * kCubemapFaceCount))
      << datas.size() << " vs " << (meta.level_num * kCubemapFaceCount);

  Texture res(meta);
  for (int level = 0; level < meta.level_num; ++level) {
    const size_t face_payload_size = static_cast<size_t>(meta.data_size_in_byte(level) / kCubemapFaceCount);
    for (int face = 0; face < kCubemapFaceCount; ++face) {
      res.SetCpuFaceLevelData(face, level, datas[TextureSubresourceIndex(meta, face, level)], face_payload_size);
    }
  }
  res.Varify();
  return res;
}

Texture CreateTexture2D(int width, int height, const std::vector<glm::vec4>& data,
                        rhi::FilterMode min_filter, rhi::FilterMode mag_filter,
                        rhi::WrapMode wrap_s, rhi::WrapMode wrap_t) {
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

  meta.format = rhi::TextureFormat::kRGBA32F;
  meta.pixel_format = rhi::PixelFormat::kRGBA;
  meta.pixel_type = rhi::PixelType::kFloat32;
  meta.min_filter = min_filter;
  meta.mag_filter = mag_filter;
  meta.wrap_s = wrap_s;
  meta.wrap_t = wrap_t;
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
  meta.format = rhi::TextureFormat::kRGBA8;
  meta.pixel_format = rhi::PixelFormat::kRGBA;
  meta.pixel_type = rhi::PixelType::kUInt8;
  meta.min_filter = rhi::FilterMode::kLinear;
  meta.mag_filter = rhi::FilterMode::kLinear;
  meta.wrap_s = rhi::WrapMode::kRepeat;
  meta.wrap_t = rhi::WrapMode::kRepeat;
  CGCHECK(meta.data_size_in_byte() == texture_2d_array.size())
      << meta.data_size_in_byte() << " " << texture_2d_array.size();
  Texture res(meta);
  res.SetCpuLevelData(0, texture_2d_array.data(), texture_2d_array.size());
  res.Varify();
  return res;
}

Texture ReadTexture(const std::string& fullpath) {
  Texture::Meta meta;
  meta = ReadTextureMeta(fullpath);
  void* data = ReadTextureData(fullpath, meta);
  Texture res = CreateTexture2D(meta, std::vector<void*>{data});
  DeleteTexture(reinterpret_cast<stbi_uc*>(data));
  res.Varify();
  return res;
}

Texture ReadTexture2DLevels(const std::vector<std::string>& fullpaths) {
  CGCHECK(!fullpaths.empty());
  Texture::Meta meta = ReadTextureMeta(fullpaths.front());
  meta.type = Texture::kTexture2D;
  meta.level_num = static_cast<int>(fullpaths.size());
  meta.depth = 1;

  Texture texture(meta);
  for (int level = 0; level < meta.level_num; ++level) {
    const Texture::Meta level_meta = ReadTextureMeta(fullpaths[level]);
    CGCHECK(level_meta.width == std::max(1, meta.width >> level)) << fullpaths[level];
    CGCHECK(level_meta.height == std::max(1, meta.height >> level)) << fullpaths[level];
    CGCHECK(level_meta.hdr == meta.hdr) << fullpaths[level];
    void* data = ReadTextureData(fullpaths[level], level_meta);
    texture.SetCpuLevelData(level, data, level_meta.data_size_in_byte());
    DeleteTexture(reinterpret_cast<stbi_uc*>(data));
  }
  texture.Varify();
  return texture;
}

Texture ReadCubemapLevels(const std::vector<std::string>& fullpaths, int level_num) {
  CGCHECK(level_num > 0) << level_num;
  CGCHECK(fullpaths.size() == static_cast<size_t>(level_num * kCubemapFaceCount))
      << fullpaths.size() << " vs " << (level_num * kCubemapFaceCount);

  Texture::Meta level0_meta = ReadTextureMeta(fullpaths.front());
  Texture::Meta meta = level0_meta;
  meta.type = Texture::kCubemap;
  meta.level_num = level_num;
  meta.depth = kCubemapFaceCount;
  meta.min_filter = level_num > 1 ? rhi::FilterMode::kLinearMipmapLinear : rhi::FilterMode::kLinear;
  meta.mag_filter = rhi::FilterMode::kLinear;
  meta.wrap_s = rhi::WrapMode::kClampToEdge;
  meta.wrap_t = rhi::WrapMode::kClampToEdge;

  Texture texture(meta);
  for (int level = 0; level < level_num; ++level) {
    const int expected_width = std::max(1, meta.width >> level);
    const int expected_height = std::max(1, meta.height >> level);
    for (int face = 0; face < kCubemapFaceCount; ++face) {
      const std::string& fullpath = fullpaths[TextureSubresourceIndex(meta, face, level)];
      const Texture::Meta face_meta = ReadTextureMeta(fullpath);
      CGCHECK(face_meta.width == expected_width) << fullpath;
      CGCHECK(face_meta.height == expected_height) << fullpath;
      CGCHECK(face_meta.hdr == meta.hdr) << fullpath;
      void* data = ReadTextureData(fullpath, face_meta);
      texture.SetCpuFaceLevelData(face, level, data, face_meta.data_size_in_byte());
      DeleteTexture(reinterpret_cast<stbi_uc*>(data));
    }
  }
  texture.Varify();
  return texture;
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

template<typename ChannelType>
std::vector<ChannelType> Texture::GetData() const {
  meta_.Varify();
  if (rhi::HasDevice() && storage_ != nullptr && storage_->uploaded_to_gl) {
    std::vector<ChannelType> data;
    data.resize(data_size_in_byte() / sizeof(ChannelType));
    rhi::GetDevice().ReadTextureData(*this,
                                     {
                                         .level = 0,
                                         .size_in_bytes = static_cast<size_t>(data_size_in_byte()),
                                     },
                                     data.data());
    return data;
  }

  CGCHECK(storage_ != nullptr && !storage_->cpu_levels.empty())
      << "Texture CPU payload is unavailable before a GPU upload/readback.";
  CGCHECK(storage_->cpu_levels[0].size() % sizeof(ChannelType) == 0)
      << "Texture payload size is not aligned to requested channel type.";

  std::vector<ChannelType> data(storage_->cpu_levels[0].size() / sizeof(ChannelType));
  if (!storage_->cpu_levels[0].empty()) {
    std::memcpy(data.data(), storage_->cpu_levels[0].data(), storage_->cpu_levels[0].size());
  }
  return data;
}

template std::vector<float> Texture::GetData<float>() const;
template std::vector<unsigned char> Texture::GetData<unsigned char>() const;

bool Texture::Varify() const {
  bool not_empty = !empty();
  CGCHECK(not_empty);
  return not_empty && meta_.Varify();
}

bool Texture::empty() const {
  return storage_ == nullptr ||
         (storage_->id == std::numeric_limits<uint32_t>::max() &&
          storage_->cpu_levels.empty() &&
          storage_->owns_gl_texture);
}

uint32_t Texture::id() const {
  EnsureUploadedToActiveBackend();
  CGCHECK(storage_ != nullptr) << "Texture storage is unavailable.";
  CGCHECK(storage_->id != std::numeric_limits<uint32_t>::max()) << "Texture used before assign id";
  return storage_->id;
}

void Texture::ReleaseGpuResources() const {
  if (storage_ == nullptr || !storage_->owns_gl_texture || storage_->id == std::numeric_limits<uint32_t>::max()) {
    return;
  }
  if (rhi::HasDevice()) {
    rhi::GetDevice().ReleaseTexture(const_cast<Texture*>(this));
  } else {
    storage_->id = std::numeric_limits<uint32_t>::max();
    storage_->uploaded_to_gl = false;
  }
}

void Texture::SetCpuLevelData(int level, const void* data, size_t size_in_bytes) {
  SetCpuFaceLevelData(0, level, data, size_in_bytes);
}

void Texture::SetCpuFaceLevelData(int face, int level, const void* data, size_t size_in_bytes) {
  CGCHECK(level >= 0) << level;
  if (storage_ == nullptr) {
    storage_ = std::make_shared<Storage>();
  }
  const size_t subresource_index = TextureSubresourceIndex(meta_, face, level);
  if (storage_->cpu_levels.size() <= subresource_index) {
    storage_->cpu_levels.resize(subresource_index + 1);
  }
  storage_->cpu_levels[subresource_index] = CopyBytes(data, size_in_bytes);
}

void Texture::EnsureUploadedToActiveBackend() const {
  CGCHECK(storage_ != nullptr) << "Texture storage is unavailable.";
  if (storage_->uploaded_to_gl) {
    return;
  }

  CGCHECK(rhi::HasDevice()) << "Texture upload requires an active RHI device.";
  rhi::GetDevice().EnsureTextureUploaded(const_cast<Texture*>(this));
}

// std::string GetCubemapPath(const std::unordered_map<std::string, std::string>& paths,
//                            int level, int texture_unit_offset) {
//   std::vector<std::string> faces = {"px", "nx", "py", "ny", "pz", "nz"};
//   std::string key = std::format("level{}_{}", level, faces[texture_unit_offset]);
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
// cg::Texture CreateCubemapPreviewTexture2DImpl(const CubemapParam& param) {
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
  CGCHECK(channel_num >= 1 && channel_num <= 4) << channel_num;
  CGCHECK(type != Texture::kUnknown) << type;
  CGCHECK(level_num != -1) << level_num;
  CGCHECK(depth != -1) << depth;
  CGCHECK(format != rhi::TextureFormat::kUnknown);
  CGCHECK(pixel_format != rhi::PixelFormat::kUnknown);
  CGCHECK(pixel_type != rhi::PixelType::kUnknown);
  return true;
}

int Texture::Meta::data_size_in_byte(int level) const {
  const int level_width = std::max(1, width >> level);
  const int level_height = std::max(1, height >> level);
  int channel_size_in_byte = hdr ? 4 : 1;   // GLfloat GLubyte
  return level_width * level_height * depth * channel_num * channel_size_in_byte;
}

int Texture::Meta::resized_data_size_in_byte(int resized_width, int resized_height, int level) const {
  int channel_size_in_byte = hdr ? 4 : 1;   // GLfloat GLubyte
  return resized_width * resized_height * depth * channel_num * channel_size_in_byte;
}

void SaveTexture(const std::string& file_name, const Texture& texture) {
  if (texture.meta().hdr) {
    std::string name = file_name + ".hdr";
    std::vector<float> data = texture.GetData<float>();
    FlipVertically(texture.meta(), data.data());
    CGCHECK(stbi_write_hdr(name.c_str(), texture.meta().width, texture.meta().height,
                           texture.meta().channel_num, data.data())) << "Save failed : " << file_name;
  } else {
    std::string name = file_name + ".png";
    std::vector<unsigned char> data = texture.GetData<unsigned char>();
    FlipVertically(texture.meta(), data.data());
    CGCHECK(stbi_write_png(name.c_str(), texture.meta().width, texture.meta().height,
                           texture.meta().channel_num, data.data(), 0)) << "Save failed : " << file_name;
  }
}

// void SaveTexture2DImpl(const std::unordered_map<std::string, std::string>& paths,
//                        const cg::Texture& texture, int level_num, bool multiple_sample = false) {
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
//                     const cg::Texture& texture, int level_num) {
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
//   CGCHECK(state->texture.type() == cg::Texture::Type::kTexture2D);
//   SaveTexture2DImpl(state->paths, state->texture, state->level_num);
// }

// void TextureRepo::SaveCubemap(const std::string& name) {
//   LOG(ERROR) << "TextureRepo::SaveCubemap " << name;
//   CGCHECK(textures_.count(name) > 0) << name;
//   State* state = &textures_[name];
//   CGCHECK(state->texture.type() == cg::Texture::Type::kCubemap);
//   const std::unordered_map<std::string, std::string>& path = state->paths;
//   SaveCubemapImpl(path, state->texture, state->level_num);
// }

// void TextureRepo::ResetTexture2D(const std::string& name, const TextureParam& param) {
//   CGCHECK(textures_.count(name) > 0) << name;
//   LOG(ERROR) << "TextureRepo::ResetTexture2D " << name;
//   State* state = &textures_[name];
//   state->texture = CreateTextureImpl(param);
//   state->loaded = true;
//   state->texture_type = cg::Texture::Texture2D;
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
//   state->texture_type = cg::Texture::Cubemap;
// }

int TextureRepo::AddUnique(const std::string& full_path) {
  if (name_2_index_.find(full_path) != name_2_index_.end()) {
    return name_2_index_.at(full_path);
  }

  int index = static_cast<int>(index_2_texture_.size());
  name_2_index_[full_path] = index;
  index_2_texture_[index] = ReadTexture(full_path);
  return index;
}

int TextureRepo::AddUnique(const std::string& name, const Texture& texture) {
  if (name_2_index_.find(name) != name_2_index_.end()) {
    return name_2_index_.at(name);
  }
  if (config_ != nullptr && config_->texture_configs().find(name) != config_->texture_configs().end()) {
    return GetIndex(name);
  }
  int index = static_cast<int>(index_2_texture_.size());
  name_2_index_[name] = index;
  index_2_texture_[index] = texture;
  return index;
}

bool TextureRepo::Has(const std::string& path) const {
  return name_2_index_.find(path) != name_2_index_.end() ||
         (config_ != nullptr && config_->texture_configs().find(path) != config_->texture_configs().end());
}

bool TextureRepo::Has(int index) const {
  CGCHECK(name_2_index_.size() == index_2_texture_.size());
  return index_2_texture_.find(index) != index_2_texture_.end();
}

int TextureRepo::GetIndex(const std::string& name) const {
  EnsureTextureLoaded(name);
  CGCHECK(name_2_index_.find(name) != name_2_index_.end()) << name;
  return name_2_index_.at(name);
}

const Texture& TextureRepo::GetTexture(const std::string& name) const {
  return GetTexture(GetIndex(name));
}

const Texture& TextureRepo::GetTexture(int index) const {
  CGCHECK(Has(index)) << index;
  return index_2_texture_.at(index);
}

int TextureRepo::size() const {
  CGCHECK(name_2_index_.size() == index_2_texture_.size());
  int compatible_texture_num = 0;
  for (const auto& entry : index_2_texture_) {
    const Texture& texture = entry.second;
    if (texture.meta().type == Texture::kTexture2D && !texture.meta().hdr) {
      ++compatible_texture_num;
    }
  }
  return compatible_texture_num;
}

const Texture& TextureRepo::AsTexture2DArray(int width, int height) const {
  bool dirty = (dirty_index_2_texture_ != index_2_texture_);
  if (dirty) {
    std::vector<Texture> textures;
    textures.reserve(index_2_texture_.size());
    for (int index = 0; index < index_2_texture_.size(); ++index) {
      const Texture& texture = index_2_texture_.at(index);
      if (texture.meta().type == Texture::kTexture2D && !texture.meta().hdr) {
        textures.push_back(texture);
      }
    }
    if (textures.empty()) {
      texture_2d_array_.ReleaseGpuResources();
      texture_2d_array_ = Texture();
    } else {
      texture_2d_array_.ReleaseGpuResources();
      texture_2d_array_ = CreateTexture2DArray(textures, width, height);
    }
    dirty_index_2_texture_ = index_2_texture_;
  }
  CGCHECK(!texture_2d_array_.empty()) << "No compatible Texture2D inputs are available for Texture2DArray.";
  texture_2d_array_.Varify();
  return texture_2d_array_;
}

void TextureRepo::EnsureTextureLoaded(const std::string& name) const {
  if (name_2_index_.find(name) != name_2_index_.end()) {
    return;
  }
  CGCHECK(config_ != nullptr) << "TextureRepo::Init must be called before named texture access: " << name;
  CGCHECK(config_->texture_configs().find(name) != config_->texture_configs().end()) << name;
  const int index = static_cast<int>(index_2_texture_.size());
  name_2_index_[name] = index;
  index_2_texture_[index] = LoadTextureFromConfig(config_->texture_config(name));
}

Texture TextureRepo::LoadTextureFromConfig(const TextureConfig& texture_config) const {
  std::vector<std::string> paths;
  paths.reserve(texture_config.path_size());
  for (const std::string& path : texture_config.path()) {
    paths.push_back(config_ == nullptr ? path : config_->ResolvePath(path));
  }

  switch (texture_config.texture_type()) {
    case ::cg::Texture2D:
      CGCHECK(static_cast<int>(paths.size()) == texture_config.level_num())
          << texture_config.name();
      return ReadTexture2DLevels(paths);
    case ::cg::Cubemap:
      return ReadCubemapLevels(paths, texture_config.level_num());
    case ::cg::Texture2DArray: {
      CGCHECK(texture_config.level_num() == 1) << texture_config.name();
      std::vector<Texture> textures;
      textures.reserve(paths.size());
      for (const std::string& path : paths) {
        textures.push_back(ReadTexture(path));
      }
      CGCHECK(!textures.empty()) << texture_config.name();
      return CreateTexture2DArray(textures, textures.front().meta().width, textures.front().meta().height);
    }
    case ::cg::Unknown:
    case ::cg::TextureType_INT_MIN_SENTINEL_DO_NOT_USE_:
    case ::cg::TextureType_INT_MAX_SENTINEL_DO_NOT_USE_:
      break;
  }
  CGCHECK(false) << "Unsupported texture config type for " << texture_config.name();
  return Texture();
}
} // namespace cg
