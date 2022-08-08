#pragma once

#include <functional>
#include <stack>

#include "base/debug.h"
#include "renderer/bvh.h"
#include "renderer/config.h"
#include "renderer/camera.h"
#include "renderer/frame_stat.h"
#include "renderer/io.h"
#include "renderer/light.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/object.h"
#include "renderer/primitive.h"
#include "renderer/inspector.h"
#include "renderer/shader.h"
#include "renderer/shadow.h"
#include "renderer/texture.h"

namespace cg {
class Scene {
 public:
  virtual ~Scene() {}
  virtual void OnEnter() = 0;
  virtual void OnUpdate() = 0;
  virtual void OnRender() = 0;
  virtual void OnExit() = 0;

  void Enter(const std::string& name, Config* config, Io* io, FrameStat* frame_stat);
  void Update();
  void Render();
  void Exit();

  const ShaderProgramRepo& shader_program_repo() const { return shader_program_repo_; }
  ShaderProgramRepo* mutable_shader_program_repo() { return &shader_program_repo_; }
  const TextureRepo& texture_repo() const { return texture_repo_; }
  const MeshRepo& mesh_repo() const { return mesh_repo_; }
  const ObjectRepo& object_repo() const { return object_repo_; }
  const MaterialRepo& material_repo() const { return material_repo_; }
  MaterialRepo* mutable_material_repo() { return &material_repo_; }
  const LightRepo& light_repo() const { return light_repo_; }
  LightRepo* mutable_light_repo() { return &light_repo_; }
  const ShadowRepo& shadwow_repo() const { return shadow_repo_; }
  const BVH& bvh() const { return bvh_; }
  const PrimitiveRepo& primitive_repo() const { return primitive_repo_; }
  
  const Camera& camera() const;
  Camera* mutable_camera();

  const FrameStat& frame_stat() const { return *frame_stat_; }
  const Io& io() const { return *io_; }

 protected:
  void MoveCamera();

  std::string name_;

  Io* io_;
  FrameStat* frame_stat_;
  Config* config_;

  std::unique_ptr<Camera> camera_ = std::make_unique<Camera>();
  
  ShaderProgramRepo shader_program_repo_;
  TextureRepo texture_repo_;
  ObjectRepo object_repo_;
  MeshRepo mesh_repo_;
  MaterialRepo material_repo_;
  PrimitiveRepo primitive_repo_;
  LightRepo light_repo_;
  ShadowRepo shadow_repo_;

  BVH bvh_;

  Inspector inspector_;
};
} // namespace cg