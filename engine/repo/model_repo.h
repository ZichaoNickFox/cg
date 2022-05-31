#pragma once

#include <memory>
#include <unordered_map>

#include <assimp/material.h>
#include <assimp/scene.h>

#include "engine/mesh.h"
#include "engine/proto/config.pb.h"
#include "engine/texture.h"

namespace engine {
constexpr char kUniformDiffuse[] = "texture_diffuse";
constexpr char kUniformSpecular[] = "texture_specular";
constexpr char kUniformNormal[] = "texture_normal";
constexpr char kUniformAmbient[] = "texture_ambient";
constexpr char kUniformHeight[] = "texture_height";
constexpr char kUniformAlbedo[] = "texture_albedo";
constexpr char kUniformMetallic[] = "texture_metallic";
constexpr char kUniformRoughness[] = "texture_roughtness";

class ModelRepo {
 public:
  void Init(const Config& config);
  struct ModelPartData {
    std::map<std::string, std::vector<engine::Texture>> uniform_2_texture;
    std::shared_ptr<engine::Mesh> mesh = std::make_shared<engine::Mesh>();
    engine::Texture texture(const std::string& uniform, int index = 0) const;
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