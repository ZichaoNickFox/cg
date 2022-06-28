#pragma once

#include <boost/any.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "renderer/debug.h"
#include "renderer/definition.h"
#include "renderer/ssbo.h"
#include "renderer/texture.h"

namespace renderer {

constexpr char kAlbedo[] = "albedo";
constexpr char kAmbient[] = "ambient";
constexpr char kDiffuse[] = "diffuse";
constexpr char kSpecular[] = "specular";
constexpr char kRoughness[] = "roughness";
constexpr char kMetallic[] = "metallic";
constexpr char kShininess[] = "shininess";
constexpr char kEmission[] = "emission";
constexpr char kTextureDiffuse[] = "texture_diffuse";
constexpr char kTextureSpecular[] = "texture_specular";
constexpr char kTextureAmbient[] = "texture_ambient";
constexpr char kTextureEmissive[] = "texture_emissive";
constexpr char kTextureHeight[] = "texture_height";
constexpr char kTextureNormal[] = "texture_normal";
constexpr char kTextureShininess[] = "texture_shininess";
constexpr char kTextureOpacity[] = "texture_opacity";
constexpr char kTextureDisplacement[] = "texture_displacemetn";
constexpr char kTextureLightmap[] = "texture_lightmap";
constexpr char kTextureReflection[] = "texture_reflection";
// pbr
constexpr char kTextureBaseColor[] = "texture_base_color";
constexpr char kTextureNormalCamera[] = "texture_normal_camera";
constexpr char kTextureEmissionColor[] = "texture_emission_color";
constexpr char kTextureMetalness[] = "texture_metalness";
constexpr char kTextureDiffuseRoughness[] = "texture_diffuse_roughness";
constexpr char kTextureAmbientOcclusion[] = "texture_ambient_occlusion";
constexpr char kTextureSheen[] = "texture_sheen";
constexpr char kTextureClearCoat[] = "texture_clear_coat";
constexpr char kTextureTransmission[] = "texture_transmission";
constexpr char kTextureUnknown[] = "texture_unknown";

struct Material {
  using Properties = std::unordered_map<std::string, boost::any>;
  Material(const Properties& properties);
  Material() = default;

  glm::vec4 albedo = glm::vec4(0, 0, 0, 0);
  glm::vec4 ambient = glm::vec4(0, 0, 0, 0);
  glm::vec4 diffuse = glm::vec4(0, 0, 0, 0);
  glm::vec4 specular = glm::vec4(0, 0, 0, 0);
  glm::vec4 emission = glm::vec4(0, 0, 0, 0);
  float roughness = 0;
  float metalness = 0;
  float shininess = 0;

  int texture_normal = -1;
  int texture_specular = -1;
  int texture_ambient = -1;
  int texture_diffuse = -1;
  int texture_base_color = -1;
  int texture_roughness = -1;
  int texture_metalness = -1;
  int texture_ambient_occlusion = -1;
  int texture_height = -1;
  int texture_shininess = -1;

  bool operator==(const Material& other) const = default;
};
  
class MaterialRepo {
 public:
  MaterialRepo();
  void Add(const std::string& name, const Material& material);
  bool Has(const std::string& name);
  const Material& GetMaterial(int material_index) const;
  const Material& GetMaterial(const std::string& name) const;
  Material* mutable_material(int material_index);
  int GetMaterialIndex(const std::string& material_name) const;
  int GetIndex(const std::string& material_name) const;
  std::string GetName(int material_index) const;
  int length() const { return index_2_material_.size(); }

  void UpdateSSBO();

 private:
  struct MaterialGPU {
    MaterialGPU() = default;  
    MaterialGPU(const Material& material);
    glm::vec4 albedo;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 emission;
    glm::vec4 roughness_metalness_shininess; // free w

    glm::vec4 texture_index_normal_specular_ambient_diffuse;
    glm::vec4 texture_index_basecolor_roughness_metalness_ambientocclusion;
    glm::vec4 texture_index_height_shininess; // free y w
  };
  SSBO ssbo_;

  std::unordered_map<int, Material> index_2_material_;
  std::unordered_map<std::string, int> name_2_index_;

  std::unordered_map<int, Material> dirty_index_2_material_;
};

}