#include "playground/object/model.h"

#include "imgui.h"

#include "engine/repo/model_repo.h"
#include "engine/util.h"

void ModelPart::OnUpdate(Context *context) {

}

void ModelPart::OnRender(Context *context) {
  if (!hidden_) {
    material_.PrepareShader();
    model_part_data_.mesh->Submit();
  }
}

void ModelPart::OnDestory(Context *context) {

}

std::optional<engine::Texture> ModelPart::texture_diffuse(int i) const {
  if (model_part_data_.diffuse_textures.size() >= i + 1) {
    return model_part_data_.diffuse_textures[i];
  }
  return {};
}

std::optional<engine::Texture> ModelPart::texture_specular(int i) const {
  if (model_part_data_.specular_textures.size() >= i + 1) {
    return model_part_data_.specular_textures[i];
  }
  return {};
}

std::optional<engine::Texture> ModelPart::texture_normal(int i) const {
  if (model_part_data_.normal_textures.size() >= i + 1) {
    return model_part_data_.normal_textures[i];
  }
  return {};
}

std::optional<engine::Texture> ModelPart::texture_ambient(int i) const {
  if (model_part_data_.ambient_textures.size() >= i + 1) {
    return model_part_data_.ambient_textures[i];
  }
  return {};
}

std::optional<engine::Texture> ModelPart::texture_height(int i) const {
  if (model_part_data_.height_textures.size() >= i + 1) {
    return model_part_data_.height_textures[i];
  }
  return {};
}

std::optional<engine::Texture> ModelPart::texture_albedo(int i) const {
  if (model_part_data_.albedo_textures.size() >= i + 1) {
    return model_part_data_.albedo_textures[i];
  }
  return {};
}

std::optional<engine::Texture> ModelPart::texture_metallic(int i) const {
  if (model_part_data_.metallic_textures.size() >= i + 1) {
    return model_part_data_.metallic_textures[i];
  }
  return {};
}

std::optional<engine::Texture> ModelPart::texture_roughness(int i) const {
  if (model_part_data_.roughness_textures.size() >= i + 1) {
    return model_part_data_.roughness_textures[i];
  }
  return {};
}

void Model::Init(Context* context, const std::string& object_name, const std::string& model_name) {
  std::vector<engine::ModelRepo::ModelPartData> model_parts_data = context->GetModel(model_name);
  for (const engine::ModelRepo::ModelPartData& model_part_data : model_parts_data) {
    model_parts_.push_back(ModelPart(model_part_data));
  }
}

void Model::ModelInspector() {
  if (ImGui::TreeNode("Mesh Parts")) {
    ImGui::PushID("Mesh Parts");
    for (int i = 0; i < model_part_num(); ++i) {
      ModelPart* model_part = &model_parts_[i];
      const engine::ModelRepo::ModelPartData& model_part_data = model_part->model_part_data();
      if (ImGui::TreeNode(util::Format("{}", model_part_data.mesh->name()).c_str())) {
        ImGui::PushID(model_part_data.mesh->name().c_str());
        ImGui::Checkbox(util::Format("hidden{}", i).c_str(), model_part->mutable_hidden());
        ImGui::Text("Position Num : %lu", model_part_data.mesh->positions().size());
        ImGui::Text("Normal Num : %lu", model_part_data.mesh->normals().size());
        ImGui::Text("Texcoord Num : %lu", model_part_data.mesh->texcoords().size());
        ImGui::Text("Tangent Num : %lu", model_part_data.mesh->tangents().size());
        ImGui::Text("Bitangent Num : %lu", model_part_data.mesh->bitangents().size());
        if (ImGui::TreeNode(util::Format("Diffuse Textures Num : {}",
                                         model_part_data.diffuse_textures.size()).c_str())) {
          for (int i = 0; i < model_part_data.diffuse_textures.size(); ++i) {
            ImGui::Text("%s", model_part_data.diffuse_textures[i].info().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Specular Textures Num : {}",
                                         model_part_data.specular_textures.size()).c_str())) {
          for (int i = 0; i < model_part_data.specular_textures.size(); ++i) {
            ImGui::Text("%s", model_part_data.specular_textures[i].info().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Normal Textures Num : {}",
                                         model_part_data.normal_textures.size()).c_str())) {
          for (int i = 0; i < model_part_data.normal_textures.size(); ++i) {
            ImGui::Text("%s", model_part_data.normal_textures[i].info().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Ambient Textures Num : {}",
                                         model_part_data.ambient_textures.size()).c_str())) {
          for (int i = 0; i < model_part_data.ambient_textures.size(); ++i) {
            ImGui::Text("%s", model_part_data.ambient_textures[i].info().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Height Textures Num : {}",
                                         model_part_data.height_textures.size()).c_str())) {
          for (int i = 0; i < model_part_data.height_textures.size(); ++i) {
            ImGui::Text("%s", model_part_data.height_textures[i].info().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Albedo Textures Num : {}",
                                         model_part_data.albedo_textures.size()).c_str())) {
          for (int i = 0; i < model_part_data.albedo_textures.size(); ++i) {
            ImGui::Text("%s", model_part_data.albedo_textures[i].info().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Roughness Textures Num : {}",
                                         model_part_data.roughness_textures.size()).c_str())) {
          for (int i = 0; i < model_part_data.roughness_textures.size(); ++i) {
            ImGui::Text("%s", model_part_data.roughness_textures[i].info().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Metallic Textures Num : {}",
                                         model_part_data.metallic_textures.size()).c_str())) {
          for (int i = 0; i < model_part_data.metallic_textures.size(); ++i) {
            ImGui::Text("%s", model_part_data.metallic_textures[i].info().c_str());
          }
          ImGui::TreePop();
        }
        ImGui::TreePop();
        ImGui::PopID();
      }
    }
    ImGui::TreePop();
    ImGui::PopID();
  }
}