#pragma once

#include <stack>

#include <glog/logging.h>

#include "engine/camera.h"
#include "playground/io.h"
#include "playground/mesh_repo.h"
#include "playground/model_repo.h"
#include "playground/proto/config.pb.h"
#include "playground/shader_repo.h"
#include "playground/texture_repo.h"

class Context {
 public:
  struct Option {
    std::string config_path;
    glm::vec4 clear_color;
  };
  void Init(const Option& option);

  void SetCurrentScene(const std::string& current_scene) { current_scene_ = current_scene; }
  const std::string& current_scene() const { return current_scene_; }

  void SetNextScene(const std::string& next_scene) { next_scene_ = next_scene; }
  const std::string& next_scene() const { return next_scene_; }

  const ShaderRepo& shader_repo() { return shader_repo_; }
  const TextureRepo& texture_repo() { return texture_repo_; }
  const MeshRepo& resh_repo() { return mesh_repo_; }
  const ModelRepo& model_repo() { return model_repo_; }
  ShaderRepo* mutable_shader_repo() { return &shader_repo_; }
  TextureRepo* mutable_texture_repo() { return &texture_repo_; }
  MeshRepo* mutable_mesh_repo() { return &mesh_repo_; }
  ModelRepo* mutable_model_repo() { return &model_repo_; }

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
  engine::Texture GetTexture(const std::string& name, bool flip_vertically = false, bool use_mipmap = false);
  std::shared_ptr<const engine::Mesh> GetMesh(const std::string& name);
  std::vector<ModelRepo::ModelPartData> GetModel(const std::string& name);

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

 private:
  std::string current_scene_;
  std::string next_scene_;
  
  ShaderRepo shader_repo_;
  TextureRepo texture_repo_;
  MeshRepo mesh_repo_;
  ModelRepo model_repo_;
  google::protobuf::Map<google::protobuf::int32, LightAttenuationConfig> light_attenuation_config_;
  google::protobuf::Map<google::protobuf::string, MaterialProperty> material_property_config_;

  Io io_;

  // Move camera outside context
  std::weak_ptr<engine::Camera> camera_;

  int frame_interval_;
  int fps_;

  glm::vec4 clear_color_;

  float camera_move_speed_ = 5;
  float camera_rotate_speed_ = 5;
};