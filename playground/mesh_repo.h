#pragma once

#include <memory>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>

#include "engine/mesh.h"
#include "engine/texture.h"
#include "playground/proto/config.pb.h"

class MeshRepo {
 public:
  void Init(const Config& config);
  std::shared_ptr<engine::Mesh> GetOrLoadMesh(const std::string& name);

 private:
  std::unordered_map<std::string, std::shared_ptr<engine::Mesh>> meshes_;
};