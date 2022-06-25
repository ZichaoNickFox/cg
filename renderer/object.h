#pragma once

#include <optional>

#include "renderer/filter.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/primitive.h"
#include "renderer/ssbo.h"
#include "renderer/transform.h"

namespace renderer {

struct ObjectMeta {
  std::string object_name;
  Transform transform;
  std::string mesh_or_model_name;
  std::string material_name;
};

struct Object {
  int object_index;
  Transform transform;
  int mesh_index;      // Only used in CPU
  int material_index;
};

struct ObjectRepo {
 public:
  void AddOrReplace(const Config& config, const std::vector<ObjectMeta>& object_metas,
                    MeshRepo* meshe_repo, MaterialRepo* material_repo, TextureRepo* texture_repo);
  void AddOrReplace(const Config& config, const ObjectMeta& object_meta,
                    MeshRepo* meshe_repo, MaterialRepo* material_repo, TextureRepo* texture_repo);
  const Object& GetObject(int object_index) const;
  const Object& GetObject(const std::string& name) const;
  Object* MutableObject(int object_index);
  Object* MutableObject(const std::string& name);
  int GetIndex(const std::string& name) const;
  std::vector<Object> GetObjects(const Filter& filter = Filter()) const;
  std::vector<Object*> MutableObjects(const Filter& filter = Filter());
  void GetPrimitives(const MeshRepo& mesh_repo, const MaterialRepo& material_repo, const Filter& filter = Filter(),
                     PrimitiveRepo* primitive_repo = nullptr);
  std::string GetName(int object_index) const;

 private:
  std::unordered_map<int, Object> index_2_object_;
  std::unordered_map<std::string, int> name_2_index_;
};

} // namespace renderer