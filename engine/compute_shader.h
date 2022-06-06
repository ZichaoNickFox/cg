#pragma once

#include <unordered_map>

#include "engine/camera.h"
#include "engine/geometry.h"
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
  ComputeShader(const Shader& shader);

  void SetWorkGroupNum(const glm::vec3& work_group_num);
  void SetTextures(const std::vector<Texture>& textures);
  void SetTextureMetas(const std::vector<TextureMeta>& texture_metas);

  void SetCamera(Camera* camera, bool with_view, bool with_project);
  void SetSpheres(const std::vector<Sphere>& spheres);
  void SetScreenSize(const glm::vec2& screen_size);

  Shader* mutable_shader() { return &shader_; }
 private:
  void CheckInternalFormat(const engine::Texture& texture);

  Shader shader_;
  glm::vec3 work_group_num_;
  std::vector<Texture> textures_;
  std::vector<TextureMeta> texture_metas_;
};

};