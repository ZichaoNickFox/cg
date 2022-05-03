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
  void OnRender(Context *context) override;
  void OnDestory(Context *context) override;

  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { return &material_; }
  std::optional<engine::Texture> texture_diffuse(int i = 0) const;
  std::optional<engine::Texture> texture_specular(int i = 0) const;
  std::optional<engine::Texture> texture_normal(int i = 0) const;
  std::optional<engine::Texture> texture_ambient(int i = 0) const;
  std::optional<engine::Texture> texture_height(int i = 0) const;

  const engine::ModelRepo::ModelPartData& model_part_data() const { return model_part_data_; }

  bool hidden() { return hidden_; }
  void SetHidden(bool hidden) { hidden_ = hidden; }
  bool* mutable_hidden() { return &hidden_; }

  std::shared_ptr<const engine::Mesh> mesh(Context* context) const override { return model_part_data_.mesh; }

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

 private:
  std::vector<ModelPart> model_parts_;
};