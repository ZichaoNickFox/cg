#pragma once

#include <optional>
#include <string>
#include <vector>

#include "engine/mesh.h"
#include "engine/repo/model_repo.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/object/object.h"

class ModelPart : public Object {
 public:
  ModelPart(const engine::ModelRepo::ModelPartData& model_part_data) : model_part_data_(model_part_data) {}
  void OnUpdate(Context *context) override;
  void OnRender(Context *context, int instance_num = 1) override;
  void OnDestory(Context *context) override;

  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { ;return &material_; }
  const engine::ModelRepo::ModelPartData& model_part_data() const { return model_part_data_; }

  bool hidden() { return hidden_; }
  void SetHidden(bool hidden) { hidden_ = hidden; }
  bool* mutable_hidden() { return &hidden_; }

  std::shared_ptr<const engine::Mesh> mesh(Context* context) const override { return model_part_data_.mesh; }
  engine::Mesh* mutable_mesh() { return model_part_data_.mesh.get(); }

 private:
  bool hidden_ = false;

  engine::ModelRepo::ModelPartData model_part_data_;
  engine::Material material_;
};

class Model {
 public:
  void Init(Context* context, const std::string& object_name, const std::string& model_name);
  void ModelInspector();

  int model_part_num() { return model_parts_.size(); }
  ModelPart* mutable_model_part(int i) { return &model_parts_[i]; }

  void OnRender(Context* context, int instance_num = 1);
  void SetTransform(const engine::Transform& transform);
  template<typename VertexAttributesType>
  void AddVertexAttribute(const engine::VertexAttribute& meta_data,
                          const std::vector<VertexAttributesType>& data) {
    for (int i = 0; i < model_parts_.size(); ++i) {
      mutable_model_part(i)->mutable_mesh()->AddVertexAttribute(meta_data, data);
    }
  }

 private:
  std::vector<ModelPart> model_parts_;
};