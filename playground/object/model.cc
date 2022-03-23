#include "engine/model_loader.h"

#include <assert.h>
#include <glog/logging.h>
#include <set>

#include "GL/glew.h"
#include "engine/util.h"

namespace model {
namespace {
void ProcessNode(aiNode* ai_node, const aiScene* ai_scene, std::vector<std::shared_ptr<Mesh>>* meshes, Material* material) {
  for(GLuint i = 0; i < ai_node->mNumMeshes; i++){
    aiMesh* ai_mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];
    ProcessMesh(ai_mesh, ai_scene, meshes);
    meshes->push_back(meshes);
  }

  for(GLuint i = 0; i < ai_node->mNumChildren; i++){
    ProcessNode(ai_node->mChildren[i], ai_scene, inoutMeshes, inoutMaterials);
  }
}

void ProcessMesh(aiMesh* ai_mesh, const aiScene* ai_scene, std::shared_ptr<Mesh> mesh){
  // position, normal, uv
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> uvs;
  for(GLuint i = 0; i < ai_mesh->mNumVertices; ++i){
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    GLuint indice;

    position.x = ai_mesh->mVertices[i].x;
    position.y = ai_mesh->mVertices[i].y;
    position.z = ai_mesh->mVertices[i].z;
    positions.push_back(position);

    if(ai_mesh->mNormals){
      normal.x = ai_mesh->mNormals[i].x;
      normal.y = ai_mesh->mNormals[i].y;
      normal.z = ai_mesh->mNormals[i].z;
      normals.push_back(normal);
    }

    // 这里临时只处理一个texture的情况
    // for(int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i){
      if(ai_mesh->mTextureCoords[0]){
        uv.x = ai_mesh->mTextureCoords[0][i].x;
        uv.y = ai_mesh->mTextureCoords[0][i].y;
        uvs.push_back(uv);
      }
    // }
  }
  mesh->SetPositions(positions);
  mesh->SetNormals(normals);
  mesh->SetUvs(uvs);

  // indices
  std::vector<GLuint> indices;
  for (GLuint i = 0; i < ai_mesh->mNumFaces; i++){
    const aiFace& f = ai_mesh->mFaces[i];
    for (GLuint j = 0; j < f.mNumIndices; j++){
      indices.push_back(f.mIndices[j]);
    }
  }
  mesh->SetIndices(indices);
  mesh->Setup();

  // textures
  std::vector<texture> ts;
  if(ai_mesh->mMaterialIndex >= 0){
    aiMaterial* aimat = ais->mMaterials[ai_mesh->mMaterialIndex];
    std::vector<texture> diffuse_textures = LoadMaterialTextures(aimat, aiTextureType_DIFFUSE);
    ts.insert(ts.end(), diffuse_textures.begin(), diffuse_textures.end());
    std::vector<texture> specular_textures = LoadMaterialTextures(aimat, aiTextureType_SPECULAR);
    ts.insert(ts.end(), specular_textures.begin(), specular_textures.end());
  }
}

std::vector<texture> LoadMaterialTextures(
  aiMaterial* aimat, 
  aiTextureType ai_texture_type
) {
  std::vector<texture> ret;
  std::set<std::string> loaded_textures;
  for(GLuint i = 0; i < aimat->GetTextureCount(ai_texture_type); ++i){
    aiString s;
    aimat->GetTexture(ai_texture_type, i, &s);
    std::string fileName = s.C_Str();
    if (util::setContains(loaded_textures, fileName)){
      continue;
    }
    texture t = texture::loadFromFile(fileName, false);
    ret.push_back(t);
  }
  return ret;
}
} // namespace

bool LoadFromFile(const std::string& path, std::vector<std::shared_ptr<Mesh>>* meshes, Material* material, bool flip_uv) {
  Assimp::Importer importer;
  const aiScene* ai_scene = importer.ReadFile(path, aiProcess_Triangulate | flip_uv ? aiProcess_FlipUVs : 0);
  if(!ai_scene || ai_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode){
    CHECK(false) << "model loadModel: Import mesh failed";
    return false;
  }
  ProcessNode(ai_scene->mRootNode, meshes inoutMeshes, material);
  return true;
}
} // namespace model