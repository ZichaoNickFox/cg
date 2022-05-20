#pragma once

#include <memory>
#include <unordered_map>

#include <assimp/material.h>
#include <assimp/scene.h>

#include "engine/mesh.h"
#include "engine/proto/config.pb.h"
#include "engine/texture.h"

namespace engine {
class ModelRepo {
 public:
  void Init(const Config& config);
  struct ModelPartData {
    std::vector<engine::Texture> ambient_textures;
    std::vector<engine::Texture> diffuse_textures;
    std::vector<engine::Texture> specular_textures;
    std::vector<engine::Texture> normal_textures;
    std::vector<engine::Texture> height_textures;

    std::vector<engine::Texture> albedo_textures;
    std::vector<engine::Texture> metallic_textures;
    std::vector<engine::Texture> roughness_textures;
    std::shared_ptr<engine::Mesh> mesh = std::make_shared<engine::Mesh>();
  };
  std::vector<ModelPartData> GetOrLoadModel(const std::string& name);

 private:
  struct State {
    bool loaded = false;
    std::string name;
    std::string mesh_path;
    std::string texture_dir;
    std::vector<ModelPartData> model_parts;

    ModelPartData* handling_model_part = nullptr;
  };
  std::vector<engine::Texture> LoadTextures(const aiMaterial& mat, aiTextureType type, State* state);
  void ProcessNode(const aiScene& scene, const aiNode& node, State* state);
  void ProcessMesh(const aiMesh& mesh, State* state);
  void ProcessTexture(const aiScene& scene, const aiMesh& ai_mesh, State* state);

  std::unordered_map<std::string, State> models_;
  std::string texture_base_dir_;
};
} // namespace engine