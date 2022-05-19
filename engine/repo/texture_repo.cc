#include "engine/repo/texture_repo.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "engine/debug.h"
#include "engine/util.h"

using engine::Texture;

namespace engine {
GLubyte* ReadImage(const std::string& filename, int* width, int* height, int* channels, int force_channels = 4) {
	GLubyte* res = stbi_load(util::ReplaceBackslash(filename).c_str(), width, height, channels, force_channels);
  CGCHECK(res) << stbi_failure_reason() << " " << util::ReplaceBackslash(filename);
	return res;
}

void DeleteImage(GLubyte* image) {
  CGCHECK(image);
  free(image);
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
    int width, height, nr_components;
    // For width, height
    CGCHECK(stbi_loadf(GetTexture2DPath(paths, 0).c_str(), &width, &height, &nr_components, 0));
    glTexStorage2D_(GL_TEXTURE_2D, level_num, GL_RGBA16F, width, height);
    for (int level = 0; level < level_num; ++level) {
      float* image = stbi_loadf(GetTexture2DPath(paths, level).c_str(), &width, &height, &nr_components, 0);
      CGCHECK(image);
      glTexSubImage2D_(GL_TEXTURE_2D, level, 0, 0, width, height, GL_RGBA, GL_FLOAT, image);
      free(image);
    }
  } else {
    int width, height;
    std::string path = GetTexture2DPath(paths, 0);
    // TODO : Just for width, height. Find a better method
    DeleteImage(ReadImage(path, &width, &height, 0, 4));
    glTexStorage2D_(GL_TEXTURE_2D, level_num, GL_RGBA8, width, height);
    for (int level = 0; level < level_num; ++level) {
      GLubyte* image = ReadImage(GetTexture2DPath(paths, level), &width, &height, 0, 4);
      CGCHECK(image);
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

Texture CreateTexture2DImpl(const CreateTexture2DParam& param) {
  GLuint ret;
  glGenTextures_(1, &ret);
  glBindTexture_(GL_TEXTURE_2D, ret);
  glTexStorage2D_(GL_TEXTURE_2D, param.level_num, param.internal_format, param.width, param.height);
  for (int level = 0; level < param.level_num; ++level) {
    glTexSubImage2D_(GL_TEXTURE_2D, level, 0, 0, param.width >> level, param.height >> level,
                    param.format, param.type, param.data->mutable_data(level));
  }
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if(param.level_num > 1){
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  glBindTexture_(GL_TEXTURE_2D, 0);
  return Texture(ret, Texture::Texture2D);
}

Texture ResetCubemapImpl(const CreateCubemapParam& param) {
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
engine::Texture CreateCubemapPreviewTexture2DImpl(const CreateCubemapParam& param) {
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

void RemoveFromGL(GLuint in) {
  glDeleteTextures_(1, &in);
}

void SaveTexture2DImpl(const std::unordered_map<std::string, std::string>& paths,
                       const engine::Texture& texture, int level_num, bool multiple_sample = false) {
  GLuint target = multiple_sample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
  glBindTexture_(target, texture.id());

  for (int level = 0; level < level_num; ++level) {
    std::string path = GetTexture2DPath(paths, level);
    int width = -1, height = -1, internal_format = -1;
    glGetTexLevelParameteriv_(target, level, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv_(target, level, GL_TEXTURE_HEIGHT, &height);
    glGetTexLevelParameteriv_(target, level, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
    
    int channel = -1, byte_per_channel = -1, format = -1, type = -1;
    GetInternalFormatSize(internal_format, &channel, &byte_per_channel, &format, &type);

    std::vector<GLubyte> pixels(width * height * channel * byte_per_channel);

    glGetTexImage_(target, level, format, type, pixels.data());
    FlipVertically(pixels.data(), width, height, channel, byte_per_channel);

    CGCHECK(width > 0) << "Widget must > 0";
    CGCHECK(height > 0) << "Height must > 0";

    CGCHECK(VarifyChannel(path, channel)); 

    TryMakeDir(path);
    SaveImage(path, width, height, channel, pixels.data());
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
  DeleteImage(ReadImage(path, &base_width, &base_height, 0, 4));

  glTexStorage2D_(GL_TEXTURE_CUBE_MAP, level_num, GL_RGBA8, base_width, base_height);
  for(int texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset) {
    for (int level = 0; level < level_num; ++level) {
      std::string cubemap_path = GetCubemapPath(paths, level, texture_unit_offset);
      int width, height;
      GLubyte* image = ReadImage(cubemap_path, &width, &height, 0, 4);
      if(!image) {
        CGCHECK(false) << util::Format("cannot load image {}", cubemap_path);
      }
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

int SaveCubemapImpl(const std::unordered_map<std::string, std::string>& paths, GLuint texture, int level_num) {
  glBindTexture_(GL_TEXTURE_CUBE_MAP, texture);

  for(int texture_unit_offset = 0; texture_unit_offset < 6; ++texture_unit_offset){
    for (int level = 0; level < level_num; ++level) {
      int width = -1, height = -1, internal_format = -1;
      int texture_unit = GL_TEXTURE_CUBE_MAP_POSITIVE_X + texture_unit_offset;
      glGetTexLevelParameteriv_(texture_unit, level, GL_TEXTURE_WIDTH, &width);
      glGetTexLevelParameteriv_(texture_unit, level, GL_TEXTURE_HEIGHT, &height);
      glGetTexLevelParameteriv_(texture_unit, level, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

      int channel = -1, byte_per_channel = -1, format = -1, type = -1;
      GetInternalFormatSize(internal_format, &channel, &byte_per_channel, &format, &type);

      std::vector<GLubyte> pixels(width * height * channel * byte_per_channel);

      glGetTexImage_(texture_unit, level, format, type, pixels.data());
      FlipVertically(pixels.data(), width, height, channel, byte_per_channel);

      std::string file_path = GetCubemapPath(paths, level, texture_unit_offset);
      TryMakeDir(file_path);
      SaveImage(file_path, width, height, channel, pixels.data());
    }
  }
  glBindTexture_(GL_TEXTURE_CUBE_MAP, 0);
  return true;
}

void TextureRepo::Init(const Config& config) {
  for (const TextureConfig& config : config.texture_config()) {
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
  CGCHECK(state->texture.type() == engine::Texture::Type::Texture2D);
  SaveTexture2DImpl(state->paths, state->texture, state->level_num);
}

void TextureRepo::SaveCubemap(const std::string& name) {
  LOG(ERROR) << "TextureRepo::SaveCubemap " << name;
  CGCHECK(textures_.count(name) > 0) << name;
  State* state = &textures_[name];
  CGCHECK(state->texture.type() == engine::Texture::Type::Cubemap);
  const std::unordered_map<std::string, std::string>& path = state->paths;
  SaveCubemapImpl(path, state->texture.id(), state->level_num);
}

void TextureRepo::ResetTexture2D(const std::string& name, const CreateTexture2DParam& param) {
  CGCHECK(textures_.count(name) > 0) << name;
  LOG(ERROR) << "TextureRepo::ResetTexture2D " << name;
  State* state = &textures_[name];
  state->texture = CreateTexture2DImpl(param);
  state->loaded = true;
  state->texture_type = engine::Texture::Texture2D;
}

Texture TextureRepo::CreateTempTexture2D(const CreateTexture2DParam& param) {
  LOG(ERROR) << "TextureRepo::CreateTempTexture2D ";
  return CreateTexture2DImpl(param);
}

Texture TextureRepo::CreateCubemapPreviewTexture2D(const CreateCubemapParam& param) {
  LOG(ERROR) << "TextureRepo::CreateCubemapPreviewTexture2D";
  return CreateCubemapPreviewTexture2DImpl(param);
}

void TextureRepo::ResetCubemap(const std::string& name, const CreateCubemapParam& param) {
  CGCHECK(textures_.count(name) > 0) << name;
  LOG(ERROR) << "TextureRepo::ResetCubemap " << name;
  State* state = &textures_[name];
  state->texture = ResetCubemapImpl(param);
  state->loaded = true;
  state->texture_type = engine::Texture::Cubemap;
}

Texture LoadModelTexture2D(const std::string& full_path, bool flip_vertically) {
  stbi_set_flip_vertically_on_load(flip_vertically);

  GLuint id;
  glGenTextures_(1, &id);
  glBindTexture_(GL_TEXTURE_2D, id);
  int width, height, channel;
  GLubyte* image = ReadImage(full_path, &width, &height, &channel, 4);
  CGCHECK(image);
  glTexImage2D_(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture_(GL_TEXTURE_2D, 0);

  DeleteImage(image);
  return Texture(id, Texture::Texture2D);
}
} // namespace engine