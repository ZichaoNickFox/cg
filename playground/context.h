#pragma once

#include <stack>

#include "engine/debug.h"
#include "engine/camera.h"
#include "engine/frame_stat.h"
#include "engine/io.h"
#include "engine/proto/config.pb.h"
#include "engine/repo/mesh_repo.h"
#include "engine/repo/model_repo.h"
#include "engine/repo/shader_repo.h"
#include "engine/repo/texture_repo.h"

class Context {
 public:
  struct Option {
    std::string config_path;
    glm::vec4 clear_color;
    glm::vec2 framebuffer_size;
  };
  void Init(const Option& option);

  const engine::ShaderRepo& shader_repo() { return shader_repo_; }
  const engine::TextureRepo& texture_repo() { return texture_repo_; }
  const engine::MeshRepo& resh_repo() { return mesh_repo_; }
  const engine::ModelRepo& model_repo() { return model_repo_; }
  engine::ShaderRepo* mutable_shader_repo() { return &shader_repo_; }
  engine::TextureRepo* mutable_texture_repo() { return &texture_repo_; }
  engine::MeshRepo* mutable_mesh_repo() { return &mesh_repo_; }
  engine::ModelRepo* mutable_model_repo() { return &model_repo_; }

  const Io& io() { return io_; }
  Io* mutable_io() { return &io_; }

  void SetCamera(engine::Camera* camera) { camera_ = camera; }
  
  const engine::Camera& camera() const;
  engine::Camera* mutable_camera();

  void StatFrame(int frame_interval);

  const glm::vec4 clear_color() const { return clear_color_; }

  engine::Shader GetShader(const std::string& name);

  engine::Texture GetTexture(const std::string& name, bool flip_vertically = false, bool equirectangular = false);
  void ResetTexture2D(const std::string& name, const engine::TextureParam& param);
  void ResetCubemap(const std::string& name, const engine::CubemapParam& param);
  engine::Texture CreateCubemapPreviewTexture2D(const engine::CubemapParam& param);
  void SaveTexture2D(const std::string& name);
  void SaveCubemap(const std::string& name);
  engine::Texture CreateTexture(const engine::TextureParam& param);

  std::shared_ptr<const engine::Mesh> GetMesh(const std::string& name);
  std::vector<engine::ModelRepo::ModelPartData> GetModel(const std::string& name);

  float* mutable_camera_move_speed() { return &camera_move_speed_; }
  float* mutable_camera_rotate_speed() { return &camera_rotate_speed_; }
  float camera_move_speed() { return camera_move_speed_; }
  float camera_rotate_speed() { return camera_rotate_speed_; }

  const glm::ivec2& framebuffer_size() const { return framebuffer_size_; }
  const engine::FrameStat& frame_stat() const { return frame_stat_; }

 private:
  engine::ShaderRepo shader_repo_;
  engine::TextureRepo texture_repo_;
  engine::MeshRepo mesh_repo_;
  engine::ModelRepo model_repo_;

  Io io_;

  // Move camera outside context
  engine::Camera* camera_ = nullptr;

  engine::FrameStat frame_stat_;

  glm::vec4 clear_color_;

  float camera_move_speed_ = 5;
  float camera_rotate_speed_ = 5;

  glm::ivec2 framebuffer_size_;
};