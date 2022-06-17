#pragma once

#include <optional>
#include <string>
#include <vector>

#include "engine/geometry.h"
#include "engine/mesh.h"
#include "engine/repo/model_repo.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/object/object.h"

class ModelPartObject : public Object {
 public:
  ModelPartObject(const engine::ModelRepo::ModelPartData& model_part_data) : model_part_data_(model_part_data) {}
  void OnUpdate(Context *context) override;
  void OnRender(Context *context, int instance_num = 1) override;
  void OnDestory(Context *context) override;

  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { ;return &material_; }
  const engine::ModelRepo::ModelPartData& model_part_data() const { return model_part_data_; }

  bool hidden() { return hidden_; }
  void SetHidden(bool hidden) { hidden_ = hidden; }
  bool* mutable_hidden() { return &hidden_; }

  std::shared_ptr<const engine::Mesh> GetMesh(Context* context) const override { return model_part_data_.mesh; }
  engine::Mesh* mutable_mesh() { return model_part_data_.mesh.get(); }

 private:
  bool hidden_ = false;

  engine::ModelRepo::ModelPartData model_part_data_;
  engine::Material material_;
};

class ModelObject : public Object {
 public:
  void Init(Context* context, const std::string& object_name, const std::string& model_name);
  void ModelInspector();

  void OnUpdate(Context *context) override;
  void OnRender(Context *context, int instance_num = 1) override;
  void OnDestory(Context *context) override;
  int material_num() const override { return model_part(0).material_num(); }
  engine::Material* mutable_material(int index = 0) override { return mutable_model_part(0)->mutable_material(); }
  std::shared_ptr<const engine::Mesh> GetMesh(Context* context) const override;

  int model_part_num() const { return model_parts_.size(); }
  ModelPartObject* mutable_model_part(int i) { return &model_parts_[i]; }
  const ModelPartObject& model_part(int i) const { return model_parts_[i]; }

  void SetTransform(const engine::Transform& transform);
  template<typename VertexAttributesType>
  void AddVertexAttribute(const engine::VertexAttribute& meta_data,
                          const std::vector<VertexAttributesType>& data) {
    for (int i = 0; i < model_parts_.size(); ++i) {
      mutable_model_part(i)->mutable_mesh()->AddVertexAttribute(meta_data, data);
    }
  }
  std::vector<engine::AABB> GetAABBs(Context* context);

 private:
  std::vector<ModelPartObject> model_parts_;
};