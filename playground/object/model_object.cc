#include "playground/object/model_object.h"

#include "imgui.h"

#include "engine/repo/model_repo.h"
#include "engine/util.h"

void ModelPartObject::OnUpdate(Context *context) {

}

void ModelPartObject::OnRender(Context *context, int instance_num) {
  if (!hidden_) {
    material_.PrepareShader();
    model_part_data_.mesh->Submit(instance_num);
  }
}

void ModelPartObject::OnDestory(Context *context) {

}

void ModelObject::Init(Context* context, const std::string& object_name, const std::string& model_name) {
  std::vector<engine::ModelRepo::ModelPartData> model_parts_data = context->GetModel(model_name);
  for (const engine::ModelRepo::ModelPartData& model_part_data : model_parts_data) {
    model_parts_.push_back(ModelPartObject(model_part_data));
  }
}

void ModelObject::ModelInspector() {
  if (ImGui::TreeNode("Mesh Parts")) {
    ImGui::PushID("Mesh Parts");
    for (int i = 0; i < model_part_num(); ++i) {
      ModelPartObject* model_part = &model_parts_[i];
      const engine::ModelRepo::ModelPartData& model_part_data = model_part->model_part_data();
      if (ImGui::TreeNode(util::Format("{}", model_part_data.mesh->name()).c_str())) {
        ImGui::PushID(model_part_data.mesh->name().c_str());
        ImGui::Checkbox(util::Format("hidden{}", i).c_str(), model_part->mutable_hidden());
        ImGui::Text("Position Num : %lu", model_part_data.mesh->positions().size());
        ImGui::Text("Normal Num : %lu", model_part_data.mesh->normals().size());
        ImGui::Text("Texcoord Num : %lu", model_part_data.mesh->texcoords().size());
        ImGui::Text("Tangent Num : %lu", model_part_data.mesh->tangents().size());
        ImGui::Text("Bitangent Num : %lu", model_part_data.mesh->bitangents().size());
        for (auto& pair : model_part_data.uniform_2_texture) {
          const std::string& uniform = pair.first;
          const std::vector<engine::Texture>& textures = pair.second;
          if (ImGui::TreeNode(util::Format("{} Textures Num : {}", uniform, textures.size()).c_str())) {
            for (int i = 0; i < textures.size(); ++i) {
              ImGui::Text("%s", textures[i].info().c_str());
            }
            ImGui::TreePop();
          }
        }
        ImGui::TreePop();
        ImGui::PopID();
      }
    }
    ImGui::PopID();
    ImGui::TreePop();
  }
}

void ModelObject::OnRender(Context* context, int instance_num) {
  for (int i = 0; i < model_part_num(); ++i) {
    mutable_model_part(i)->OnRender(context, instance_num);
  }
}

void ModelObject::SetTransform(const engine::Transform& transform) {
  for (int i = 0; i < model_part_num(); ++i) {
    mutable_model_part(i)->SetTransform(transform);
  }
}