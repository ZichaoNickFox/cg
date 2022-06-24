#include "playground/scene/model_scene.h"

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>
#include <vector>

#include "renderer/gl.h"
#include "renderer/pass.h"
#include "renderer/shaders.h"
#include "renderer/util.h"

using namespace renderer;

void ModelScene::OnEnter() {
  camera_->mutable_transform()->SetTranslation(glm::vec3(0.329854, 2.252055, 1.987056));
  camera_->mutable_transform()->SetRotation(glm::quat(0.908184, {-0.401313, 0.110919, 0.042983}));

  object_repo_.AddOrReplace(config_, object_metas_, &mesh_repo_, &material_repo_, &texture_repo_);
  light_repo_.Add(lights);
}

void ModelScene::OnUpdate() {
  ImGui::Checkbox("texture_ambient", &use_texture_ambient_);
  ImGui::Checkbox("texture_diffuse", &use_texture_diffuse_);
  ImGui::Checkbox("texture_normal", &use_texture_normal_);
  ImGui::Checkbox("texture_height", &use_texture_height_);
  ImGui::Checkbox("texture_specular", &use_texture_specular_);
  ImGui::Checkbox("show_vertex_normal", &show_vertex_normal_);
  ImGui::Checkbox("show_texture_normal", &show_texture_normal_);
  ImGui::Checkbox("show_TBN", &show_TBN_);
  ImGui::Checkbox("show_triangle", &show_triangle_);
  ImGui::SliderFloat("shininess", &shininess_, 0, 1);
  ImGui::Checkbox("use_blinn_phong", &use_blinn_phong_);
  ImGui::SliderFloat("rotate speed", &rotate_speed_, 0.0, 0.1);

  ImGui::Separator();

  ImGui::Checkbox("Cull", &enable_cull_face_);
  if (ImGui::Button("Back")) {
    call_face_ = GL_BACK;
  }
  if (ImGui::Button("Front")) {
    call_face_ = GL_FRONT;
  }
  if (ImGui::Button("Front And Back")) {
    call_face_ = GL_FRONT_AND_BACK;
  }
  if (ImGui::Button("CW")) {
    cw_ = GL_CW;
  }
  if (ImGui::Button("CCW")) {
    cw_ = GL_CCW;
  }

  glm::quat rotation = glm::angleAxis(rotate_speed_, glm::vec3(0, 1, 0));
  for (Object* object : object_repo_.MutableObjects()) {
    object->transform.SetRotation(rotation);
  }
}

void ModelScene::OnRender() {
  if (enable_cull_face_) {
    glEnable_(GL_CULL_FACE);
  } else {
    glDisable_(GL_CULL_FACE);
  }
  glCullFace_(call_face_);
  glFrontFace_(cw_);

  PhongShader({true}, *this, object_repo_.GetObject("teapot"));
  // LinesShader({}, *this, CoordinatorMesh());
}

void ModelScene::OnExit() {
}