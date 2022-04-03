#pragma once

#include <string>
#include <vector>

#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/model_repo.h"
#include "playground/object/object.h"

class Model : public Object {
 public:
  void Init(Context* context, const std::string& object_name, const std::string& model_name);
  void OnUpdate(Context *context) override;
  void OnRender(Context *context) override;
  void OnDestory(Context *context) override;

  engine::Material* mutable_materials(int index) { return &model_materials_[index]; }

  void ModelInspector();

  int model_part_num() { return model_parts_.size(); }

 private:
  std::vector<ModelRepo::ModelPart> model_parts_;
  std::vector<engine::Material> model_materials_;
};