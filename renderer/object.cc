#include "renderer/object.h"

#include "renderer/model_loader.h"

namespace renderer {
void ObjectRepo::Init(const Config* config, MeshRepo* mesh_repo, MaterialRepo* material_repo,
                      TextureRepo* texture_repo) {
  config_ = config;
  mesh_repo_ = mesh_repo;
  material_repo_ = material_repo;
  texture_repo_ = texture_repo;
}

void ObjectRepo::AddOrReplace(const std::vector<ObjectMeta>& object_metas) {
  for (const ObjectMeta& object_meta : object_metas) {
    AddOrReplace(object_meta);
  }
}

void ObjectRepo::AddOrReplace(const ObjectMeta& object_meta) {
  if (!mesh_repo_->Has(object_meta.mesh_or_model_name)) {
    LoadModel(*config_, object_meta.mesh_or_model_name, mesh_repo_, material_repo_, texture_repo_);
    CGCHECK(material_repo_->Has(object_meta.material_name)) << " After LoadModel, materials should be loaded name~"
        << object_meta.material_name;
  }
  CGCHECK(material_repo_->Has(object_meta.material_name)) << " Cannot find material name~"
      << object_meta.material_name;
  int index;
  if (name_2_index_.find(object_meta.object_name) == name_2_index_.end()) {
    index = name_2_index_.size();
  } else {
    index = name_2_index_[object_meta.object_name];
  }
  name_2_index_[object_meta.object_name] = index;
  index_2_object_[index] = Object{index, object_meta.transform, mesh_repo_->GetIndex(object_meta.mesh_or_model_name),
                                  material_repo_->GetIndex(object_meta.material_name)};
}

bool ObjectRepo::Has(const std::string& object_name) const {
  return name_2_index_.find(object_name) != name_2_index_.end();
}

bool ObjectRepo::Has(int object_index) const {
  return index_2_object_.find(object_index) != index_2_object_.end();
}

const Object& ObjectRepo::GetObject(int object_index) const {
  CGCHECK(Has(object_index)) << object_index;
  return index_2_object_.at(object_index);
}

const Object& ObjectRepo::GetObject(const std::string& name) const {
  CGCHECK(Has(name));
  return GetObject(GetIndex(name));
}

Object* ObjectRepo::MutableObject(int object_index) {
  CGCHECK(Has(object_index)) << object_index;
  return &index_2_object_[object_index];
}

Object* ObjectRepo::MutableObject(const std::string& name) {
  CGCHECK(Has(name)) << name;
  return &index_2_object_[name_2_index_[name]];
}

int ObjectRepo::GetIndex(const std::string& name) const {
  CGCHECK(Has(name)) << name;
  return name_2_index_.at(name);
}

std::vector<Object> ObjectRepo::GetObjects(const Filter& filter) const {
  CGCHECK(name_2_index_.size() == index_2_object_.size());
  std::vector<Object> res(name_2_index_.size());
  for (const auto& p : name_2_index_) {
    const std::string& name = p.first;
    int index = p.second;
    if (filter.Pass(name)) {
      res[index] = index_2_object_.at(index);
    }
  }
  Object empty;
  util::Remove(empty, &res);
  return res;
}

std::vector<Object*> ObjectRepo::MutableObjects(const Filter& filter) {
  CGCHECK(name_2_index_.size() == index_2_object_.size());
  std::vector<Object*> res(name_2_index_.size(), (Object*)nullptr);
  for (const auto& p : name_2_index_) {
    const std::string& name = p.first;
    int index = p.second;
    if (filter.Pass(name)) {
      res[index] = &index_2_object_[index];
    }
  }
  util::Remove((Object*)nullptr, &res);
  return res;
}

void ObjectRepo::BreakIntoPrimitives(const MeshRepo& mesh_repo, const MaterialRepo& material_repo, const Filter& filter,
                                     PrimitiveRepo* primitive_repo) {
  int primitive_start_index = 0;
  for (Object* object : MutableObjects()) {
    const Transform transform = object->transform;
    object->primitive_start_index = primitive_start_index;
    int primitive_num = mesh_repo.BreakIntoPrimitives(object->mesh_index, object->material_index, transform,
                                                      primitive_repo);
    primitive_start_index += primitive_num;
  }
}

std::string ObjectRepo::GetName(int object_index) const {
  CGCHECK(Has(object_index)) << object_index;
  for (const auto& p : name_2_index_) {
    if (p.second == object_index) {
      return p.first;
    }
  }
  CGCHECK(false) << "Cannot find object name of index : " << object_index;
  for (const auto& p : name_2_index_) {
    CGLOG(ERROR) << p.first << " " << p.second;
  }
  return "";
}
}
