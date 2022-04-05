#include "playground/object/model.h"

#include "imgui.h"

#include "playground/model_repo.h"
#include "playground/util.h"

void Model::Init(Context* context, const std::string& object_name, const std::string& model_name) {
  model_parts_ = context->GetModel(model_name);
  model_materials_.resize(model_parts_.size());
}

void Model::OnUpdate(Context *context) {

}

void Model::OnRender(Context *context) {
  for (int i = 0; i < model_parts_.size(); ++i) {
    const ModelRepo::ModelPart& model_part = model_parts_[i];
    for (int j = 0; j < model_part.diffuse_textures.size(); ++j) {
      const engine::Texture& texture = model_part.diffuse_textures[j];
      model_materials_[i].SetTexture(util::Format("diffuse_texture{}", j), texture);
    }
    for (int j = 0; j < model_part.specular_textures.size(); ++j) {
      const engine::Texture& texture = model_part.specular_textures[j];
      model_materials_[i].SetTexture(util::Format("specular_texture{}", j), texture);
    }
    for (int j = 0; j < model_part.normal_textures.size(); ++j) {
      const engine::Texture& texture = model_part.normal_textures[j];
      model_materials_[i].SetTexture(util::Format("noraml_texture{}", j), texture);
    }
    for (int j = 0; j < model_part.ambient_textures.size(); ++j) {
      const engine::Texture& texture = model_part.ambient_textures[j];
      model_materials_[i].SetTexture(util::Format("ambient_texture{}", j), texture);
    }
  }

  for (int i = 0; i < model_parts_.size(); ++i) {
    const engine::Camera& camera = context->camera();
    glm::mat4 project = camera.GetProjectMatrix();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = transform_.GetModelMatrix();
    model_materials_[i].SetMat4("project", project);
    model_materials_[i].SetMat4("view", view);
    model_materials_[i].SetMat4("model", model);
    model_materials_[i].PrepareShader();

    model_parts_[i].mesh->Submit();
  }
}

void Model::OnDestory(Context *context) {

}

void Model::ModelInspector() {
  if (ImGui::TreeNode("Mesh Parts")) {
    ImGui::PushID("Mesh Parts");
    for (int i = 0; i < model_parts_.size(); ++i) {
      const ModelRepo::ModelPart& model_part = model_parts_[i];
      if (ImGui::TreeNode(util::Format("{}", model_part.mesh->name()).c_str())) {
        ImGui::PushID(model_part.mesh->name().c_str());
        ImGui::Text("Position Num : %lu", model_part.mesh->positions().size());
        ImGui::Text("Normal Num : %lu", model_part.mesh->normals().size());
        ImGui::Text("Texcoord Num : %lu", model_part.mesh->texcoords().size());
        ImGui::Text("Tangent Num : %lu", model_part.mesh->tangents().size());
        ImGui::Text("Bitangent Num : %lu", model_part.mesh->bitangents().size());
        ImGui::Text("Color Num : %lu", model_part.mesh->colors().size());
        if (ImGui::TreeNode(util::Format("Diffuse Textures Num : {}", model_part.diffuse_textures.size()).c_str())) {
          for (int i = 0; i < model_part.diffuse_textures.size(); ++i) {
            ImGui::Text("%s", model_part.diffuse_textures[i].name().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Specular Textures Num : {}", model_part.specular_textures.size()).c_str())) {
          for (int i = 0; i < model_part.specular_textures.size(); ++i) {
            ImGui::Text("%s", model_part.specular_textures[i].name().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Normal Textures Num : {}", model_part.normal_textures.size()).c_str())) {
          for (int i = 0; i < model_part.normal_textures.size(); ++i) {
            ImGui::Text("%s", model_part.normal_textures[i].name().c_str());
          }
          ImGui::TreePop();
        }
        if (ImGui::TreeNode(util::Format("Ambient Textures Num : {}", model_part.ambient_textures.size()).c_str())) {
          for (int i = 0; i < model_part.ambient_textures.size(); ++i) {
            ImGui::Text("%s", model_part.ambient_textures[i].name().c_str());
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