#pragma once

#include <stack>

#include <glog/logging.h>

#include "engine/camera.h"
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

  void SetCurrentScene(const std::string& current_scene) { current_scene_ = current_scene; }
  const std::string& current_scene() const { return current_scene_; }

  void SetNextScene(const std::string& next_scene) { next_scene_ = next_scene; }
  const std::string& next_scene() const { return next_scene_; }

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

  void SetCamera(std::shared_ptr<engine::Camera> camera) { camera_ = camera; }
  
  const engine::Camera& camera();
  engine::Camera* mutable_camera();

  void SetFrameInternal(int frame_interval);
  int frame_interval() const { return frame_interval_; }
  int fps() const { return fps_; }

  const glm::vec4 clear_color() const { return clear_color_; }

  engine::Shader GetShader(const std::string& name);

  engine::Texture GetTexture(const std::string& name, bool flip_vertically = false, bool equirectangular = false);
  void ResetTexture2D(const std::string& name, const engine::CreateTexture2DParam& param);
  void ResetCubemap(const std::string& name, const engine::CreateCubemapParam& param);
  void SaveTexture2D(const std::string& name);
  void SaveCubemap(const std::string& name);
  engine::Texture CreateTempTexture2D(const engine::CreateTexture2DParam& param);

  std::shared_ptr<const engine::Mesh> GetMesh(const std::string& name);
  std::vector<engine::ModelRepo::ModelPartData> GetModel(const std::string& name);

  float light_attenuation_constant(int metre);
  float light_attenuation_linear(int metre);
  float light_attenuation_quadratic(int metre);

  glm::vec4 material_property_ambient(const std::string& name);
  glm::vec4 material_property_diffuse(const std::string& name);
  glm::vec4 material_property_specular(const std::string& name);
  float material_property_shininess(const std::string& name);

  float* mutable_camera_move_speed() { return &camera_move_speed_; }
  float* mutable_camera_rotate_speed() { return &camera_rotate_speed_; }
  float camera_move_speed() { return camera_move_speed_; }
  float camera_rotate_speed() { return camera_rotate_speed_; }

  const glm::ivec2& framebuffer_size() const { return framebuffer_size_; }

 private:
  std::string current_scene_;
  std::string next_scene_;
  
  engine::ShaderRepo shader_repo_;
  engine::TextureRepo texture_repo_;
  engine::MeshRepo mesh_repo_;
  engine::ModelRepo model_repo_;
  std::unordered_map<int, LightAttenuationConfig> light_attenuation_config_;
  std::unordered_map<std::string, MaterialProperty> material_property_config_;

  Io io_;

  // Move camera outside context
  std::weak_ptr<engine::Camera> camera_;

  int frame_interval_;
  int fps_;

  glm::vec4 clear_color_;

  float camera_move_speed_ = 5;
  float camera_rotate_speed_ = 5;

  glm::ivec2 framebuffer_size_;
};