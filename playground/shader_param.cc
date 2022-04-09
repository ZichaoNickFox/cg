#include "playground/shader_param.h"

#include "engine/debug.h"
#include "playground/util.h"

namespace shader_param {
namespace {
void UpdateLightInfo(Context* context, const ShaderLightInfo& light_info, engine::Material* material) {
  CGCHECK(light_info.light_poses.size() == light_info.light_colors.size())
      << light_info.light_poses.size() << " " << light_info.light_colors.size();
  CGCHECK(light_info.light_poses.size() == light_info.light_attenuation_metres.size())
      << light_info.light_poses.size() << " " << light_info.light_attenuation_metres.size();
  material->SetInt("light_count", light_info.light_poses.size());
  for (int i = 0; i < light_info.light_poses.size(); ++i) {
    material->SetVec3(util::Format("lights[{}].color", i).c_str(), light_info.light_colors[i]);
    material->SetVec3(util::Format("lights[{}].pos", i).c_str(), light_info.light_poses[i]);
    int light_attenuation_metre = light_info.light_attenuation_metres[i];
    material->SetFloat(util::Format("lights[{}].constant", i).c_str(),
                                    context->light_attenuation_constant(light_attenuation_metre));
    material->SetFloat(util::Format("lights[{}].linear", i).c_str(),
                                    context->light_attenuation_linear(light_attenuation_metre));
    material->SetFloat(util::Format("lights[{}].quadratic", i).c_str(),
                                    context->light_attenuation_quadratic(light_attenuation_metre));
  }
  material->SetInt("light_count", light_info.light_poses.size());
}
}

void UpdateShader(Context* context, const PhongShaderParam& phong, engine::Material* material) {
  material->PushShader(context->GetShader(phong.shader_name));

  material->SetVec3("view_pos", context->camera().transform().translation());

  const std::string& material_property_name = phong.material_propery_name;
  material->SetVec3("material.ambient", context->material_property_ambient(material_property_name));
  material->SetVec3("material.diffuse", context->material_property_diffuse(material_property_name));
  material->SetVec3("material.specular", context->material_property_specular(material_property_name));
  material->SetFloat("material.shininess", context->material_property_shininess(material_property_name));

  UpdateLightInfo(context, phong.light_info, material);

  material->SetInt("blinn_phong", phong.use_blinn_phong);
}

void UpdateShader(Context* context, const PbrShaderParam& pbr, engine::Material* material) {
  material->PushShader(context->GetShader(pbr.shader_name));

  material->SetVec3("view_pos", context->camera().transform().translation());

  material->SetVec3("material.albedo", pbr.albedo);
  material->SetFloat("material.roughness", pbr.roughness);
  material->SetFloat("material.metallic", pbr.metallic);

  material->SetFloat("ao", pbr.ao);

  UpdateLightInfo(context, pbr.light_info, material);
}

};