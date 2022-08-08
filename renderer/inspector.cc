#include "renderer/inspector.h"

#include <format>
#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "base/color.h"
#include "base/debug.h"
#include "renderer/io.h"
#include "renderer/mesh/lines_mesh.h"
#include "renderer/scene.h"
#include "renderer/shader.h"

namespace cg {
void Inspector::Inspect(const std::string& scene_name, Scene* scene) {
  bool open = true;
  ImGui::Begin(scene_name.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize);

  GuiFps(scene);
  ImGui::Separator();

  InspectCamera(scene);
  ImGui::Separator();

  InSpectCursor(scene);
  ImGui::Separator();

  ShowCoordinators(scene);
  ImGui::Separator();

  ReloadShaderPrograms(scene);
  ImGui::Separator();

  InspectNormal(scene);
  ImGui::Separator();

  InspectObjects(scene);
  ImGui::Separator();

  InspectLights(scene);
  ImGui::Separator();

  InspectMaterials(scene);
  ImGui::Separator();

  ImGui::End();
}

void Inspector::InspectCamera(Scene* scene) {
  std::string camera_pos_ws = glm::to_string(scene->camera().transform().translation());
  ImGui::Text("camera_pos_ws %s", camera_pos_ws.c_str());
  ImGui::SameLine();
  if (ImGui::Button("Copy##camera_pos_ws")) {
    scene->io().write_clipboard_func()(camera_pos_ws.c_str());
  }

  std::string camera_direction_ws = glm::to_string(scene->camera().front_ws());
  ImGui::Text("camera_direction_ws %s", camera_direction_ws.c_str());
  ImGui::SameLine();
  if (ImGui::Button("Copy##camera_direction_ws")) {
    scene->io().write_clipboard_func()(camera_direction_ws.c_str());
  }

  std::string camera_rotation_quat = glm::to_string(scene->camera().transform().rotation());
  ImGui::Text("camera_rotation %s", camera_rotation_quat.c_str());
  ImGui::SameLine();
  if (ImGui::Button("Copy##camera_roatation_quat")) {
    scene->io().write_clipboard_func()(camera_rotation_quat.c_str());
  }

  glm::vec3 near_pos_ws, far_pos_ws;
  scene->camera().GetPickRay(scene->io().GetCursorPosSS(), &near_pos_ws, &far_pos_ws);
  ImGui::Text("camera_near_pos_ws %s", glm::to_string(near_pos_ws).c_str());
  ImGui::Text("camera_far_pos_ws %s", glm::to_string(far_pos_ws).c_str());
  ImGui::Text("pick dir %s", glm::to_string(glm::normalize(far_pos_ws - near_pos_ws)).c_str());

  ImGui::SliderFloat("camera move speed", scene->mutable_camera()->mutable_move_speed(), 1, 10);
  ImGui::SliderFloat("rotate speed", scene->mutable_camera()->mutable_rotate_speed(), 1, 10);
}

void Inspector::InSpectCursor(Scene* scene) {
  ImGui::Text("cursor pos x %lf", scene->io().GetCursorWindowPos().x);
  ImGui::Text("cursor pos y %lf", scene->io().GetCursorWindowPos().y);
  ImGui::Text("cursor screen space x %lf", scene->io().GetCursorPosSS().x);
  ImGui::Text("cursor screen space y %lf", scene->io().GetCursorPosSS().y);
}

void Inspector::ShowCoordinators(Scene* scene) {
  ImGui::Checkbox("show view coordinator", &show_view_coordinator_);
  ImGui::Checkbox("show world coordinator", &show_world_coordinator_);
  if (show_view_coordinator_) {
    glDisable_(GL_DEPTH_TEST);
    glm::vec3 near_pos_ws, far_pos_ws;
    scene->camera().GetPickRay(glm::vec2(0.5, 0.1), &near_pos_ws, &far_pos_ws);
    glm::vec3 direction = glm::normalize(far_pos_ws - near_pos_ws);
    glm::vec3 world_coord_pos = near_pos_ws + direction * glm::vec3(0.5, 0.5, 0.5);

    LinesShader({0.3}, *scene, CoordinatorMesh(), {world_coord_pos, glm::quat(), glm::vec3(0.05, 0.05, 0.05)});

    glEnable_(GL_DEPTH_TEST);
  }
  if (show_world_coordinator_) {
    LinesShader({}, *scene, CoordinatorMesh());
  }
}

void Inspector::ReloadShaderPrograms(Scene* scene) {
  if (ImGui::Button("Reload Shaders")) {
    scene->mutable_shader_program_repo()->ReloadShaderPrograms();
  }
}

void Inspector::InspectMesh(Scene* scene, const Object& object) {
  const std::string& mesh_name = scene->mesh_repo().GetName(object.mesh_index);
  const Mesh* mesh = scene->mesh_repo().GetMesh(object.mesh_index);
  if (ImGui::TreeNode(std::format("mesh : index ~ {} name ~ {}", object.mesh_index, mesh_name).c_str())) {
    ImGui::PushID(mesh_name.c_str());
    ImGui::Text("Position Num : %lu", mesh->positions().size());
    ImGui::Text("Normal Num : %lu  Maybe all (0,0,0)", mesh->normals().size());
    ImGui::Text("Texcoord Num : %lu  Maybe all (0,0)", mesh->texcoords().size());
    ImGui::Text("Tangent Num : %lu  Maybe all (0,0,0)", mesh->tangents().size());
    ImGui::Text("Bitangent Num : %lu  Maybe all (0,0,0)", mesh->bitangents().size());
    ImGui::TreePop();
    ImGui::PopID();
  }
}

void Inspector::InspectMaterial(int material_index, const std::string& material_name, Material* material) {
  if (ImGui::TreeNode(std::format("material index ~ {} name ~ {}", material_index, material_name).c_str())) {
    ImGui::PushID(material_name.c_str());
    ImGui::ColorEdit4("albedo", glm::value_ptr(material->albedo));
    ImGui::ColorEdit4("ambient", glm::value_ptr(material->ambient));
    ImGui::ColorEdit4("diffuse", glm::value_ptr(material->diffuse));
    ImGui::ColorEdit4("specular", glm::value_ptr(material->specular));
    ImGui::ColorEdit4("emission", glm::value_ptr(material->emission));

    ImGui::SliderFloat("roughness", &material->roughness, 0, 1);
    ImGui::SliderFloat("metalness", &material->metalness, 0, 1);
    ImGui::SliderFloat("shininess", &material->shininess, 0, 1);
    ImGui::Text("texture_normal : %d", material->texture_normal);
    ImGui::Text("texture_specular : %d", material->texture_specular);
    ImGui::Text("texture_ambient : %d", material->texture_ambient);
    ImGui::Text("texture_diffuse : %d", material->texture_diffuse);
    ImGui::Text("texture_basecolor : %d", material->texture_base_color);
    ImGui::Text("texture_roughness : %d", material->texture_roughness);
    ImGui::Text("texture_metalness : %d", material->texture_metalness);
    ImGui::Text("texture_ambientocclusion : %d", material->texture_ambient_occlusion);
    ImGui::Text("texture_height : %d", material->texture_height);
    ImGui::Text("texture_shininess : %d", material->texture_shininess);
    ImGui::TreePop();
    ImGui::PopID();
  }
}

void Inspector::InspectNormal(Scene* scene) {
  ImGui::Checkbox("show triangle", &normal_shader_param_.show_triangle);
  ImGui::Checkbox("show face normal", &normal_shader_param_.show_face_normal);
  ImGui::Checkbox("show vetex normal", &normal_shader_param_.show_vertex_normal);
  ImGui::Checkbox("show texture normal", &normal_shader_param_.show_vertex_texture_normal);
  ImGui::Checkbox("show TBN", &normal_shader_param_.show_TBN);
  ImGui::SliderFloat("length", &normal_shader_param_.length, 0, 1);
  ImGui::SliderFloat("width", &normal_shader_param_.width, 0, 5);
  for (const Object& object : scene->object_repo().GetObjects()) {
    NormalShader(normal_shader_param_, *scene, object);
  }
}

void Inspector::InspectObjects(Scene* scene) {
  if (ImGui::TreeNode("Objects")) {
    ImGui::PushID("Objects");
    for (const Object& object : scene->object_repo().GetObjects()) {
      std::string object_name = scene->object_repo().GetName(object.object_index);
      if (ImGui::TreeNode(std::format("Object : index ~ {} name ~ {}",
                          object.object_index, object_name).c_str())) {
        InspectMesh(scene, object);

        const std::string& material_name = scene->material_repo().GetName(object.material_index);
        Material* material = scene->mutable_material_repo()->mutable_material(object.material_index);
        InspectMaterial(object.material_index, material_name, material);

        ImGui::TreePop();
      }
    }
    ImGui::PopID();
    ImGui::TreePop();
  }
}

void Inspector::InspectLights(Scene* scene) {
  if (ImGui::TreeNode("Lights")) {
    ImGui::PushID("Lights");
    for (int i = 0; i < scene->light_repo().num(); ++i) {
      Light* light = scene->mutable_light_repo()->mutable_light(i);
      if (ImGui::TreeNode(std::format("Light {}", i).c_str())) {
        ImGui::PushID(std::format("{}", i).c_str());
        if (ImGui::RadioButton("DirectionalLight", light->type)) { light->type = Light::kDirectionalLight; }
        ImGui::SameLine();
        if (ImGui::RadioButton("PointLight", light->type)) { light->type = Light::kPointLight; }
        ImGui::SameLine();
        if (ImGui::RadioButton("SpotLight", light->type)) { light->type = Light::kSpotLight; }
        ImGui::SliderFloat3("pos", glm::value_ptr(light->position), -5, 5);
        ImGui::ColorEdit4("color", glm::value_ptr(light->color));
        ImGui::SliderFloat3("attenuation_2_1_0", glm::value_ptr(light->attenuation_2_1_0), 0, 1);

        ImGui::PopID();
        ImGui::TreePop();
      }
    }
    ImGui::PopID();
    ImGui::TreePop();
  }
}

void Inspector::InspectMaterials(Scene* scene) {
  if (ImGui::TreeNode("Materials")) {
    ImGui::PushID("Materials");
    for (int i = 0; i < scene->material_repo().num(); ++i) {
      Material* material = scene->mutable_material_repo()->mutable_material(i);
      InspectMaterial(i, scene->material_repo().GetName(i), material);
    }
    ImGui::PopID();
    ImGui::TreePop();
  }
}

void Inspector::GuiFps(Scene* scene) {
  scene->frame_stat().Gui();
}

RaytracingDebugCommon::LightPath::LightPath() {
  for (int i = 0; i < 20; ++i) {
    light_path[i] = glm::vec4(9999, 9999, 9999, 1);
  }
}

RaytracingDebugCommon::RaytracingDebugCommon(const Texture& fullscreen_texture, const Scene& scene,
                                             const LightPath& light_path) {
  FullscreenQuadShader({fullscreen_texture}, scene);

  glDisable_(GL_DEPTH_TEST);

  std::vector<glm::vec4> colors{kRed, kOrange, kYellow, kGreen,
                                kCyan, kBlue, kPurple, kWhite,
                                kWhite, kWhite, kWhite, kWhite,
                                kWhite, kWhite, kWhite, kWhite,
                                kWhite, kWhite, kWhite, kWhite};
  LinesShader({}, scene, LinesMesh{util::AsVector(light_path.light_path), colors, GL_LINE_STRIP});
  LinesShader({}, scene, CoordinatorMesh());

  glEnable_(GL_DEPTH_TEST);
}
} // namespace cg
