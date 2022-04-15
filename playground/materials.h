#pragma once

#include <optional>

#include "engine/material.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/lines.h"
#include "playground/object/object.h"
#include "playground/object/point_light.h"

class ShaderLightInfo {
 public:
  ShaderLightInfo() {}
  ShaderLightInfo(const PointLight& point_light);
  ShaderLightInfo(const std::vector<PointLight>& point_lights);

  void UpdateMaterial(Context* context, engine::Material* material) const;

 private:
  void Insert(const PointLight& point_light);

  std::vector<glm::vec3> light_poses;
  std::vector<glm::vec3> light_colors;
  std::vector<int> light_attenuation_metres;
};
  
class PhongShader {
 public:
  struct Param {
    std::string shader_name = "phong";
    ShaderLightInfo light_info;
    bool use_blinn_phong;

    glm::vec3 ambient = glm::vec3(0, 0, 0);
    glm::vec3 diffuse = glm::vec3(0, 0, 0);
    glm::vec3 specular = glm::vec3(0, 0, 0);
    float shininess = 0;
    std::optional<engine::Texture> texture_ambient;
    std::optional<engine::Texture> texture_normal;
    std::optional<engine::Texture> texture_specular;
    std::optional<engine::Texture> texture_diffuse;
  };
  PhongShader(const Param& param, Context* context, Object* object);
};

class PbrShader {
 public:
  struct Param {
    std::string shader_name = "pbr";
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    ShaderLightInfo light_info;

    std::optional<engine::Texture> texture_normal;
    std::optional<engine::Texture> texture_albedo;
    std::optional<engine::Texture> texture_metallic;
    std::optional<engine::Texture> texture_roughness;

  };
  PbrShader(const Param& param, Context* context, Object* object);
};

class NormalShader {
 public:
  struct Param {
    float length = 0.4;
    float width = 1.0;
    bool show_normal = true;
    bool show_TBN = true;
    bool show_triangle = true;
  };
  NormalShader(const Param& param, Context* context, Object* object);
};

class LinesShader {
 public:
  struct Param {
    float line_width = 1.0;
  };
  LinesShader(const Param& param, Context* context, Lines* object);
};