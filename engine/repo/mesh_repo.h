#pragma once

#include <memory>
#include <unordered_map>

#include "engine/mesh.h"
#include "engine/proto/config.pb.h"
#include "engine/texture.h"

namespace engine {
class MeshRepo {
 public:
  void Init(const Config& config);
  std::shared_ptr<const engine::Mesh> GetOrLoadMesh(const std::string& name);

 private:
  std::unordered_map<std::string, std::shared_ptr<const engine::Mesh>> meshes_;
};
} // namespace engine