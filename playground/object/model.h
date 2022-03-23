#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>
#include <memory>

#include "engine/material.h"
#include "engine/mesh.h"
#include "engine/texture.h"

class Model {
 public:
  
  
 private:
  engine::Material material;
  std::vector<std::shared_ptr<Mesh>> meshes_;
};

namespace model {

bool LoadFromFile(const std::string& path, std::vector<std::shared_ptr<Mesh>>* meshes, Material* material, bool flip_uv);

} // namspace model