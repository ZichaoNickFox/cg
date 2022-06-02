#pragma once

#include <unordered_map>

#include "engine/shader.h"
#include "engine/texture.h"

namespace engine {

class ComputeShader {
 public:
  struct TextureMeta {
    std::string uniform;
    int texture_unit;
    GLuint read_write_type = GL_READ_ONLY;
    GLuint internal_format = GL_RGBA32F;
  };
  void Run();

 protected:
  void SetShader(const Shader& shader) { shader_ = shader; }
  void SetWorkGroupNum(const glm::vec3& work_group_num);
  void SetTextures(const std::vector<Texture>& textures);
  void SetTextureMetas(const std::vector<TextureMeta>& texture_metas);

 private:
  void CheckInternalFormat(const engine::Texture& texture);

  Shader shader_;
  glm::vec3 work_group_num_;
  std::vector<Texture> textures_;
  std::vector<TextureMeta> texture_metas_;
};

};