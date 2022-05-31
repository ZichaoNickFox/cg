#include "engine/repo/model_repo.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <set>

#include "engine/debug.h"
#include "engine/repo/texture_repo.h"
#include "engine/util.h"

namespace engine {
engine::Texture ModelRepo::ModelPartData::texture(const std::string& uniform, int index) const {
  return uniform_2_texture.at(uniform).at(index);
}

void ModelRepo::Init(const Config& config) {
  for (const ModelConfig& config : config.model_config()) {
    State state;
    state.name = config.name();
    state.mesh_path = config.mesh_path();
    state.texture_dir = config.texture_dir();
    models_[config.name()] = state;
    CGLOG(ERROR) << "Init model : " << config.name();
  }
}

std::vector<ModelRepo::ModelPartData> ModelRepo::GetOrLoadModel(const std::string& name) {
  CGCHECK(models_.count(name) > 0) << "Cannot find model : " << name;
  State* state = &models_[name];
  if (!state->loaded) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(state->mesh_path, aiProcess_Triangulate | aiProcess_GenSmoothNormals
                                             | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      CGCHECK(false) << "AssImp Error : " << importer.GetErrorString();
      return state->model_parts;
    }
    ProcessNode(*scene, *scene->mRootNode, state);

    state->loaded = true;
    CGLOG(ERROR) << "Loaded Model : name=" << name;
  }
  for (int i = 0; i < state->model_parts.size(); ++i) {
    ModelPartData* model_part = &state->model_parts[i];
    model_part->mesh->Setup();
  }
  return state->model_parts;
}

void ModelRepo::ProcessNode(const aiScene& ai_scene, const aiNode& ai_node, State* state) {
  for(uint32_t i = 0; i < ai_node.mNumMeshes; i++) {
    aiMesh* ai_mesh = ai_scene.mMeshes[ai_node.mMeshes[i]];

    state->model_parts.push_back(ModelPartData());
    state->handling_model_part = &state->model_parts.back();
    state->handling_model_part->mesh->SetName(ai_mesh->mName.C_Str());
    
    ProcessMesh(*ai_mesh, state);
    ProcessTexture(ai_scene, *ai_mesh, state);
  }
  for(uint32_t i = 0; i < ai_node.mNumChildren; i++) {
    ProcessNode(ai_scene, *ai_node.mChildren[i], state);
  }
}

void ModelRepo::ProcessMesh(const aiMesh& ai_mesh, State* state) {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> tangents;
  std::vector<glm::vec3> bitangents;
  for(uint32_t i = 0; i < ai_mesh.mNumVertices; i++) {
    positions.push_back(glm::vec3(ai_mesh.mVertices[i].x, ai_mesh.mVertices[i].y, ai_mesh.mVertices[i].z));
    if (ai_mesh.HasNormals()) {
      normals.push_back(glm::vec3(ai_mesh.mNormals[i].x, ai_mesh.mNormals[i].y, ai_mesh.mNormals[i].z));
    }
    if(ai_mesh.mTextureCoords[0]) {
      texcoords.push_back(glm::vec2(ai_mesh.mTextureCoords[0][i].x, ai_mesh.mTextureCoords[0][i].y));
      tangents.push_back(glm::vec3(ai_mesh.mTangents[i].x, ai_mesh.mTangents[i].y, ai_mesh.mTangents[i].z));
      bitangents.push_back(glm::vec3(ai_mesh.mBitangents[i].x, ai_mesh.mBitangents[i].y, ai_mesh.mBitangents[i].z));
    } else {
      texcoords.push_back(glm::vec2(0, 0));
    }
  }
  std::vector<GLuint> indices;
  for(GLuint i = 0; i < ai_mesh.mNumFaces; i++) {
    aiFace face = ai_mesh.mFaces[i];
    for(GLuint j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);        
  }
  CGLOG(ERROR) << "Process Mesh : model=" << state->name << " mesh=" << ai_mesh.mName.C_Str();
  CGLOG(ERROR) << "Vertex Format Begin";
  CGLOG_IF(ERROR, positions.size() > 0) << "position : size=" << positions.size();
  CGLOG_IF(ERROR, normals.size() > 0) << "normals : size=" << normals.size();
  CGLOG_IF(ERROR, texcoords.size() > 0) << "texcoord : size=" << texcoords.size();
  CGLOG_IF(ERROR, tangents.size() > 0) << "tangent : size=" << tangents.size();
  CGLOG_IF(ERROR, bitangents.size() > 0) << "bitangent : size=" << bitangents.size();
  CGLOG(ERROR) << "Vertex Format End";
  std::shared_ptr<engine::Mesh> mesh = state->handling_model_part->mesh;
  mesh->SetPositions(std::move(positions));
  mesh->SetNormals(std::move(normals));
  mesh->SetTexcoords(std::move(texcoords));
  mesh->SetTangent(std::move(tangents));
  mesh->SetBitangent(std::move(bitangents));
  mesh->SetIndices(std::move(indices));
}

void ModelRepo::ProcessTexture(const aiScene& ai_scene, const aiMesh& ai_mesh, State* state) {
  const aiMaterial& ai_material = *CGCHECK_NOTNULL(ai_scene.mMaterials[ai_mesh.mMaterialIndex]);

  ModelPartData* handling_model_part = state->handling_model_part;

  std::map<aiTextureType, std::string> ai_2_uniform = {
    {aiTextureType_DIFFUSE, kUniformDiffuse},
    {aiTextureType_SPECULAR, kUniformSpecular},
    {aiTextureType_NORMALS, kUniformNormal},
    {aiTextureType_HEIGHT, kUniformHeight},
    {aiTextureType_AMBIENT, kUniformAmbient},
    {aiTextureType_BASE_COLOR, kUniformAlbedo},
    {aiTextureType_DIFFUSE_ROUGHNESS, kUniformRoughness},
    {aiTextureType_METALNESS, kUniformMetallic}
  };
  for (auto& pair : ai_2_uniform) {
    aiTextureType ai_type = pair.first;
    std::string uniform = pair.second;
    std::vector<engine::Texture> textures = LoadTextures(ai_material, ai_type, state);
    handling_model_part->uniform_2_texture[uniform].insert(
        handling_model_part->uniform_2_texture[uniform].begin(), textures.begin(), textures.end());
  }

  CGLOG(ERROR) << "Process Texture : model=" << state->name;
  CGLOG(ERROR) << "Texture Begin";
  std::vector<std::string> uniforms = {kUniformAmbient, kUniformDiffuse,
      kUniformSpecular, kUniformNormal, kUniformHeight};
  for (const std::string& uniform : uniforms) {
    int uniform_texture_size = handling_model_part->uniform_2_texture[uniform].size();
    CGLOG_IF(ERROR, uniform_texture_size > 0) << "{} : size=" << uniform << uniform_texture_size;
  }
  CGLOG(ERROR) << "Texture End";
}

std::vector<Texture> ModelRepo::LoadTextures(const aiMaterial& ai_material, aiTextureType type, State* state) {
  std::vector<engine::Texture> textures;
  std::unordered_map<std::string, engine::Texture> loaded_paths;
  for(uint32_t i = 0; i < ai_material.GetTextureCount(type); i++) {
    aiString path;
    ai_material.GetTexture(type, i, &path);
    std::string full_path = util::Format("{}/{}", state->texture_dir, path.C_Str());
    bool skip = false;
    if(loaded_paths.find(full_path) != loaded_paths.end()) {
      textures.push_back(loaded_paths[full_path]);
      skip = true;
    }
    if(!skip) {
      engine::Texture texture;
      texture = LoadModelTexture2D(full_path);
      LOG(ERROR) << "Loading " << aiTextureTypeToString(type) << " texture from " << full_path;
      textures.push_back(texture);
      loaded_paths[full_path] = texture;
    }
  }
  return textures;
}
} // namepace engine