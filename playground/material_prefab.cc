#include "playground/material_prefab.h"

#include "engine/debug.h"
#include "playground/util.h"

namespace material_prefab {

void UpdatePhongMaterial(Context* context, const PhongMaterialPrefab& phong, engine::Material* material) {
  material->PushShader(context->GetShader("phong"));

  material->SetVec3("view_pos", context->camera().transform().translation());

  const std::string& material_property_name = phong.material_propery_name;
  material->SetVec3("material.ambient", context->material_property_ambient(material_property_name));
  material->SetVec3("material.diffuse", context->material_property_diffuse(material_property_name));
  material->SetVec3("material.specular", context->material_property_specular(material_property_name));
  material->SetFloat("material.shininess", context->material_property_shininess(material_property_name));

  CGCHECK(phong.material_light_info.light_poses.size() == phong.material_light_info.light_colors.size())
      << phong.material_light_info.light_poses.size() << " " << phong.material_light_info.light_colors.size();
  CGCHECK(phong.material_light_info.light_poses.size() == phong.material_light_info.light_attenuation_metres.size())
      << phong.material_light_info.light_poses.size() << " " << phong.material_light_info.light_attenuation_metres.size();
  material->SetInt("light_count", phong.material_light_info.light_poses.size());
  for (int i = 0; i < phong.material_light_info.light_poses.size(); ++i) {
    material->SetVec3(util::Format("lights[{}].color", i).c_str(), phong.material_light_info.light_colors[i]);
    material->SetVec3(util::Format("lights[{}].pos", i).c_str(), phong.material_light_info.light_poses[i]);
    int light_attenuation_metre = phong.material_light_info.light_attenuation_metres[i];
    material->SetFloat(util::Format("lights[{}].constant", i).c_str(),
                                    context->light_attenuation_constant(light_attenuation_metre));
    material->SetFloat(util::Format("lights[{}].linear", i).c_str(),
                                    context->light_attenuation_linear(light_attenuation_metre));
    material->SetFloat(util::Format("lights[{}].quadratic", i).c_str(),
                                    context->light_attenuation_quadratic(light_attenuation_metre));
  }
  material->SetInt("light_count", phong.material_light_info.light_poses.size());
  material->SetInt("blinn_phong", phong.material_light_info.use_blinn_phong);
}

};