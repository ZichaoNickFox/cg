#include "renderer/material.h"

#include <glm/gtx/string_cast.hpp>

#include "base/debug.h"
#include "base/util.h"

namespace cg {

static const std::unordered_map<std::string, Material::Properties> kDefaultMaterialProperties = {
  {"gold", {{kAmbient, glm::vec4(0.24725, 0.1995, 0.0745, 1)},
            {kDiffuse, glm::vec4(0.75164, 0.60648, 0.22648, 1)},
            {kSpecular, glm::vec4(0.628281, 0.555802, 0.366065, 1)},
            {kShininess, 51.2}}},
  {"silver", {{kAmbient, glm::vec4(0.19225, 0.19225, 0.19225, 1)},
              {kDiffuse, glm::vec4(0.50754, 0.50754, 0.50754, 1)},
              {kSpecular, glm::vec4(0.508273, 0.508273, 0.508273, 1)}, {kShininess, 51.2}}},
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
  if (properties.count(kEmission) > 0) {
    emission = boost::any_cast<glm::vec4>(properties.at(kEmission));
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

MaterialRepo::MaterialRepo() : ssbo_(SSBO_MATERIAL_REPO) {
  for (const auto& p : kDefaultMaterialProperties) {
    const std::string& material_name = p.first;
    const Material::Properties& material_properties = p.second;
    Add(material_name, Material(material_properties));
  }
}

MaterialRepo::MaterialGPU::MaterialGPU(const Material& material) {
  albedo = material.albedo;
  ambient = material.ambient;
  diffuse = material.diffuse;
  specular = material.specular;
  emission = material.emission;
  roughness_metalness_shininess = glm::vec4(material.roughness, material.metalness, material.shininess, 0.0);
  texture_index_normal_specular_ambient_diffuse = glm::vec4(material.texture_normal, material.texture_specular,
                                                            material.texture_ambient, material.texture_diffuse);
  texture_index_basecolor_roughness_metalness_ambientocclusion = glm::vec4(
    material.texture_base_color, material.texture_roughness, material.texture_metalness,
    material.texture_ambient_occlusion);
  texture_index_height_shininess = glm::vec4(material.texture_height, material.texture_shininess, 0.0, 0.0);
}

void MaterialRepo::UpdateSSBO() {
  bool dirty = !(index_2_material_ == dirty_index_2_material_);
  if (dirty) {
    std::vector<MaterialGPU> material_gpus(index_2_material_.size());
    for (int i = 0; i < index_2_material_.size(); ++i) {
      material_gpus[i] = MaterialGPU(index_2_material_[i]);
    }
    ssbo_.SetData(util::VectorSizeInByte(material_gpus), material_gpus.data());
    dirty_index_2_material_ = index_2_material_;
  }
}

void MaterialRepo::Add(const std::string& name, const Material& material) {
  CGCHECK(!Has(name));
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

const Material& MaterialRepo::GetMaterial(const std::string& material_name) const {
  CGCHECK(name_2_index_.find(material_name) != name_2_index_.end());
  return index_2_material_.at(name_2_index_.at(material_name));
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
  CGCHECK(false) << "Cannot find material name of index : " << material_index;
  for (const auto& p : name_2_index_) {
    CGLOG(ERROR) << p.first << " " << p.second;
  }
  return "";
}
}