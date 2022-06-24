#include "renderer/material.h"

#include "renderer/debug.h"
#include "renderer/util.h"

namespace renderer {

static const std::unordered_map<std::string, Material::Properties> kDefaultMaterialProperties = {
  {"gold", {{kAmbient, glm::vec4(0.24725, 0.1995, 0.0745, 1)},
            {kDiffuse, glm::vec4(0.75164, 0.60648, 0.22648, 1)},
            {kSpecular, glm::vec4(0.628281, 0.555802, 0.366065, 1)},
            {kShininess, 51.2}}},
  {"silver", {{kAmbient, glm::vec4(0.19225, 0.19225, 0.19225, 1)},
              {kDiffuse, glm::vec4(0.50754, 0.50754, 0.50754, 1)},
              {kSpecular, glm::vec4(0.508273, 0.508273, 0.508273, 1)},
              {kShininess, 51.2}}},
  {"jade", {{kAmbient, glm::vec4(0.135, 0.2225, 0.1575, 0.95)},
            {kDiffuse, glm::vec4(0.54, 0.89, 0.63, 0.95)},
            {kSpecular, glm::vec4(0.316228, 0.316228, 0.316228, 0.95)},
            {kShininess, 12.8}}},
  {"ruby", {{kAmbient, glm::vec4(0.1745, 0.01175, 0.01175, 0.55)},
            {kDiffuse, glm::vec4(0.61424, 0.04136, 0.04136, 0.55)},
            {kSpecular, glm::vec4(0.727811, 0.626959, 0.626959, 0.55)},
            {kShininess, 76.8}}},
};

Material::Material(const Properties& properties) {
  if (properties.count(kAlbedo) > 0) {
    albedo = boost::any_cast<glm::vec4>(properties.at(kAlbedo));
  }
  if (properties.count(kAmbient) > 0) {
    ambient = boost::any_cast<glm::vec4>(properties.at(kAmbient));
  }
  if (properties.count(kDiffuse) > 0) {
    diffuse = boost::any_cast<glm::vec4>(properties.at(kDiffuse));
  }
  if (properties.count(kSpecular) > 0) {
    specular = boost::any_cast<glm::vec4>(properties.at(kSpecular));
  }
  if (properties.count(kRoughness) > 0) {
    roughness = boost::any_cast<double>(properties.at(kRoughness));
  }
  if (properties.count(kMetallic) > 0) {
    metalness = boost::any_cast<double>(properties.at(kMetallic));
  }
  if (properties.count(kShininess) > 0) {
    shininess = boost::any_cast<double>(properties.at(kShininess));
  }
  if (properties.count(kTextureNormal) > 0) {
    texture_normal = boost::any_cast<int>(properties.at(kTextureNormal));
  }
  if (properties.count(kTextureSpecular) > 0) {
    texture_specular = boost::any_cast<int>(properties.at(kTextureSpecular));
  }
  if (properties.count(kTextureAmbient) > 0) {
    texture_ambient = boost::any_cast<int>(properties.at(kTextureAmbient));
  }
  if (properties.count(kTextureDiffuse) > 0) {
    texture_diffuse = boost::any_cast<int>(properties.at(kTextureDiffuse));
  }
  if (properties.count(kTextureBaseColor) > 0) {
    texture_base_color = boost::any_cast<int>(properties.at(kTextureBaseColor));
  }
  if (properties.count(kTextureDiffuseRoughness) > 0) {
    texture_roughness = boost::any_cast<int>(properties.at(kTextureDiffuseRoughness));
  }
  if (properties.count(kTextureMetalness) > 0) {
    texture_metalness = boost::any_cast<int>(properties.at(kTextureMetalness));
  }
  if (properties.count(kTextureAmbientOcclusion) > 0) {
    texture_ambient_occlusion = boost::any_cast<int>(properties.at(kTextureAmbientOcclusion));
  }
  if (properties.count(kTextureHeight) > 0) {
    texture_height = boost::any_cast<int>(properties.at(kTextureHeight));
  }
  if (properties.count(kTextureShininess) > 0) {
    texture_shininess = boost::any_cast<int>(properties.at(kTextureShininess));
  }
}

MaterialRepo::MaterialRepo() {
  for (const auto& p : kDefaultMaterialProperties) {
    const std::string& material_name = p.first;
    const Material::Properties& material_properties = p.second;
    Add(material_name, Material(material_properties));
  }
}

void MaterialRepo::BindSSBO(int binding_point) {
  std::vector<MaterialGPU> material_gpus(index_2_material_.size());
  for (int i = 0; i < index_2_material_.size(); ++i) {
    CGCHECK(index_2_material_.find(i) != index_2_material_.end());
    const Material& material = index_2_material_.at(i);
    MaterialGPU* material_gpu = &material_gpus[i];
    material_gpu->albedo = material.albedo;
    material_gpu->ambient = material.ambient;
    material_gpu->diffuse = material.diffuse;
    material_gpu->specular = material.specular;
    material_gpu->roughness_metalness_shininess.x = material.roughness;
    material_gpu->roughness_metalness_shininess.y = material.metalness;
    material_gpu->roughness_metalness_shininess.z = material.shininess;
    material_gpu->texture_index_normal_specular_ambient_diffuse.x = material.texture_normal;
    material_gpu->texture_index_normal_specular_ambient_diffuse.y = material.texture_specular;
    material_gpu->texture_index_normal_specular_ambient_diffuse.z = material.texture_ambient;
    material_gpu->texture_index_normal_specular_ambient_diffuse.w = material.texture_diffuse;
    material_gpu->texture_index_basecolor_roughness_metalness_ambientocclusion.x = material.texture_base_color;
    material_gpu->texture_index_basecolor_roughness_metalness_ambientocclusion.y = material.texture_roughness;
    material_gpu->texture_index_basecolor_roughness_metalness_ambientocclusion.z = material.texture_metalness;
    material_gpu->texture_index_basecolor_roughness_metalness_ambientocclusion.w = material.texture_ambient_occlusion;
    material_gpu->texture_index_height_shininess.x = material.texture_height;
    material_gpu->texture_index_height_shininess.y = material.texture_shininess;
  }
  ssbo_.Init(binding_point, util::VectorSizeInByte(material_gpus), material_gpus.data());
}

void MaterialRepo::Add(const std::string& name, const Material& material) {
  CGCHECK(index_2_material_.size() == name_2_index_.size());
  int index = name_2_index_.size();
  name_2_index_[name] = index;
  index_2_material_[index] = material;
}

bool MaterialRepo::Has(const std::string& name) {
  return name_2_index_.find(name) != name_2_index_.end();
}

const Material& MaterialRepo::GetMaterial(int material_index) const {
  CGCHECK(index_2_material_.find(material_index) != index_2_material_.end());
  return index_2_material_.at(material_index);
}

Material* MaterialRepo::mutable_material(int material_index) {
  CGCHECK(index_2_material_.find(material_index) != index_2_material_.end());
  return &index_2_material_[material_index];
}

int MaterialRepo::GetMaterialIndex(const std::string& material_name) const {
  CGCHECK(name_2_index_.find(material_name) != name_2_index_.end());
  return name_2_index_.at(material_name);
}

int MaterialRepo::GetIndex(const std::string& material_name) const {
  CGCHECK(name_2_index_.find(material_name) != name_2_index_.end());
  return name_2_index_.at(material_name);
}

std::string MaterialRepo::GetName(int material_index) const {
  for (const auto& p : name_2_index_) {
    if (p.second == material_index) {
      return p.first;
    }
  }
  CGCHECK(false) << "Cannot find mesh name of index : " << material_index;
  for (const auto& p : name_2_index_) {
    CGLOG(ERROR) << p.first << " " << p.second;
  }
  return "";
}
}