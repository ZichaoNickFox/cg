#include "renderer/model_loader.h"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <memory>

#include "renderer/debug.h"
#include "renderer/object.h"

namespace renderer {

namespace {
std::unique_ptr<Mesh> ProcessMesh(const aiMesh& ai_mesh) {
  std::unique_ptr<Mesh> res = std::make_unique<Mesh>();
  for(uint32_t i = 0; i < ai_mesh.mNumVertices; i++) {
    res->mutable_positions()->push_back(glm::vec3(ai_mesh.mVertices[i].x, ai_mesh.mVertices[i].y, ai_mesh.mVertices[i].z));
    if (ai_mesh.HasNormals()) {
      res->mutable_normals()->push_back(glm::vec3(ai_mesh.mNormals[i].x, ai_mesh.mNormals[i].y, ai_mesh.mNormals[i].z));
    } else {
      res->mutable_normals()->push_back(glm::vec3(0, 0, 0));
    }
    if (ai_mesh.HasTextureCoords(0)) {
      res->mutable_texcoords()->push_back(glm::vec2(ai_mesh.mTextureCoords[0][i].x, ai_mesh.mTextureCoords[0][i].y));
    } else {
      res->mutable_texcoords()->push_back(glm::vec2(0, 0));
    }
    if (ai_mesh.HasTangentsAndBitangents()) {
      res->mutable_tangents()->push_back(glm::vec3(ai_mesh.mTangents[i].x, ai_mesh.mTangents[i].y, ai_mesh.mTangents[i].z));
      res->mutable_bitangents()->push_back(glm::vec3(ai_mesh.mBitangents[i].x, ai_mesh.mBitangents[i].y, ai_mesh.mBitangents[i].z));
    } else {
      res->mutable_tangents()->push_back(glm::vec3(0, 0, 0));
      res->mutable_bitangents()->push_back(glm::vec3(0, 0, 0));
    }
  }
  std::vector<GLuint> indices;
  for(GLuint i = 0; i < ai_mesh.mNumFaces; i++) {
    aiFace face = ai_mesh.mFaces[i];
    for(GLuint j = 0; j < face.mNumIndices; j++)
      res->mutable_indices()->push_back(face.mIndices[j]);        
  }
  res->Setup();
  CGLOG(ERROR) << "Process Mesh : mesh=" << ai_mesh.mName.C_Str();
  CGLOG(ERROR) << "Vertex Format Begin";
  CGLOG_IF(ERROR, res->positions().size() > 0) << "position : size=" << res->positions().size();
  CGLOG_IF(ERROR, res->normals().size() > 0) << "normals : size=" << res->normals().size();
  CGLOG_IF(ERROR, res->texcoords().size() > 0) << "texcoord : size=" << res->texcoords().size();
  CGLOG_IF(ERROR, res->tangents().size() > 0) << "tangent : size=" << res->tangents().size();
  CGLOG_IF(ERROR, res->bitangents().size() > 0) << "bitangent : size=" << res->bitangents().size();
  CGLOG_IF(ERROR, res->indices().size() > 0) << "indices : size=" << res->indices().size();
  CGLOG(ERROR) << "Vertex Format End";
  return res;
}

std::vector<int> LoadTextures(const std::string& model_dir, const aiMaterial& ai_material, aiTextureType type,
                              TextureRepo* texture_repo) {
  std::vector<int> res;
  for(uint32_t i = 0; i < ai_material.GetTextureCount(type); i++) {
    aiString path;
    ai_material.GetTexture(type, i, &path);
    std::string full_path = util::FileJoin(model_dir, path.C_Str());
    CGLOG(ERROR) << "Texture " << aiTextureTypeToString(type) << " : " << full_path;
    int index = texture_repo->AddUnique(full_path);
    res.push_back(index);
  }
  return res;
}

Material ProcessMaterial(const std::string& model_dir, const aiScene& ai_scene, const aiMesh& ai_mesh,
                         TextureRepo* texture_repo) {
  CGCHECK(ai_scene.mNumMaterials > 0) << "scene material == 0";
  std::unordered_map<std::string, boost::any> material_properties;
  const aiMaterial& ai_material = *CGCHECK_NOTNULL(ai_scene.mMaterials[ai_mesh.mMaterialIndex]);

  aiColor3D color; ai_material.Get(AI_MATKEY_COLOR_DIFFUSE, color);
  material_properties[kDiffuse] = glm::vec4(color.r, color.g, color.b, 1.0);
  aiVector3D ambient; ai_material.Get(AI_MATKEY_COLOR_AMBIENT, ambient);
  material_properties[kAmbient] = glm::vec4(ambient.x, ambient.y, ambient.z, 1.0);
  aiVector3D albedo; ai_material.Get(AI_MATKEY_BASE_COLOR, albedo);
  material_properties[kAlbedo] = glm::vec4(albedo.x, albedo.y, albedo.z, 1.0);
  aiVector3D specular; ai_material.Get(AI_MATKEY_COLOR_SPECULAR, specular);
  material_properties[kSpecular] = glm::vec4(specular.x, specular.y, specular.z, 1.0);
  aiVector3D emission; ai_material.Get(AI_MATKEY_COLOR_EMISSIVE, emission);
  material_properties[kEmission] = glm::vec4(emission.x, emission.y, emission.z, 1.0);
  double roughness; ai_material.Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
  material_properties[kRoughness] = roughness;
  double metallic; ai_material.Get(AI_MATKEY_METALLIC_FACTOR, metallic);
  material_properties[kMetallic] = metallic;
  double shininess; ai_material.Get(AI_MATKEY_SHININESS, shininess);
  material_properties[kShininess] = shininess;

  std::map<aiTextureType, std::string> texture_properties = {
    {aiTextureType_DIFFUSE, kTextureDiffuse},
    {aiTextureType_SPECULAR, kTextureSpecular},
    {aiTextureType_AMBIENT, kTextureAmbient},
    {aiTextureType_EMISSIVE, kTextureEmissive},
    {aiTextureType_HEIGHT, kTextureHeight},
    {aiTextureType_NORMALS, kTextureNormal},
    {aiTextureType_SHININESS, kTextureShininess},
    {aiTextureType_OPACITY, kTextureOpacity},
    {aiTextureType_DISPLACEMENT, kTextureDisplacement},
    {aiTextureType_LIGHTMAP, kTextureLightmap},
    {aiTextureType_REFLECTION, kTextureReflection},

    {aiTextureType_BASE_COLOR, kTextureBaseColor},
    {aiTextureType_NORMAL_CAMERA, kTextureNormalCamera},
    {aiTextureType_EMISSION_COLOR, kTextureEmissionColor},
    {aiTextureType_METALNESS, kTextureMetalness},
    {aiTextureType_DIFFUSE_ROUGHNESS, kTextureDiffuseRoughness},
    {aiTextureType_AMBIENT_OCCLUSION, kTextureAmbientOcclusion},
    {aiTextureType_SHEEN, kTextureSheen},
    {aiTextureType_CLEARCOAT, kTextureClearCoat},
    {aiTextureType_TRANSMISSION, kTextureTransmission},
    {aiTextureType_UNKNOWN, kTextureUnknown}
  };
  for (auto& pair : texture_properties) {
    aiTextureType ai_type = pair.first;
    std::string material_property_key = pair.second;
    std::vector<int> texture_indices = LoadTextures(model_dir, ai_material, ai_type, texture_repo);
    if (texture_indices.size() > 0) {
      CGCHECK(texture_indices.size() == 1) << " A kind of texture of a model should be 1? " << texture_indices.size();
      material_properties[material_property_key] = texture_indices[0];
    }
  }
  return Material(material_properties);
}

void ProcessNode(const std::string& model_dir, const std::string& model_name, const aiScene& ai_scene,
                 const aiNode& ai_node, MeshRepo* mesh_repo, MaterialRepo* material_repo, TextureRepo* texture_repo) {
  for(uint32_t i = 0; i < ai_node.mNumMeshes; i++) {
    aiMesh* ai_mesh = ai_scene.mMeshes[ai_node.mMeshes[i]];
    mesh_repo->Add(model_name, ProcessMesh(*ai_mesh));
    Material material = ProcessMaterial(model_dir, ai_scene, *ai_mesh, texture_repo);
    material_repo->Add(model_name, material);
  }
  for(uint32_t i = 0; i < ai_node.mNumChildren; i++) {
    ProcessNode(model_dir, model_name, ai_scene, *ai_node.mChildren[i], mesh_repo, material_repo, texture_repo);
  }
}
}

bool LoadModel(const Config& config, const std::string& model_name, MeshRepo* mesh_repo, MaterialRepo* material_repo,
               TextureRepo* texture_repo) {
  ModelConfig model_config = config.model_config(model_name);
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(util::FileJoin(model_config.model_dir(), model_config.model_file()),
                                           aiProcess_Triangulate | aiProcess_GenSmoothNormals
                                           | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    CGCHECK(false) << "AssImp Error : " << importer.GetErrorString();
    return false;
  }
  ProcessNode(model_config.model_dir(), model_name, *scene, *scene->mRootNode, mesh_repo, material_repo, texture_repo);

  CGLOG(ERROR) << "Loaded Model : name=" << model_name;
  return true;
}

} // namespace renderer