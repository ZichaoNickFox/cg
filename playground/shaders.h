#pragma once

#include <optional>
#include <glm/glm.hpp>

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/object.h"
#include "playground/object/point_light.h"

class ShaderShadowInfo {
 public:
  ShaderShadowInfo() {}
  ShaderShadowInfo(const glm::mat4& light_space_vp, const engine::Texture& texture_depth);

  void UpdateMaterial(Context* context, engine::Material* material) const;

 private:
  glm::mat4 light_space_vp_;
  engine::Texture texture_depth;
};

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
    glm::vec3 ambient = glm::vec3(0, 0, 0);
    glm::vec3 diffuse = glm::vec3(0, 0, 0);
    glm::vec3 specular = glm::vec3(0, 0, 0);
    float shininess = 0;
    std::optional<engine::Texture> texture_ambient;
    std::optional<engine::Texture> texture_normal;
    std::optional<engine::Texture> texture_specular;
    std::optional<engine::Texture> texture_diffuse;

    bool use_blinn_phong = false;

    ShaderLightInfo light_info;
    std::optional<ShaderShadowInfo> shadow_info;
  };
  PhongShader(const Param& param, Context* context, Object* object);
};

class PbrShader {
 public:
  struct Param {
    glm::vec3 albedo = glm::vec3(1, 0, 0);
    float metallic = 0.5;
    float roughness = 0.5;
    glm::vec3 ao = glm::vec3(1.0, 1.0, 1.0);

    std::optional<engine::Texture> texture_normal;
    std::optional<engine::Texture> texture_albedo;
    std::optional<engine::Texture> texture_metallic;
    std::optional<engine::Texture> texture_roughness;
    std::optional<engine::Texture> texture_ao;

    ShaderLightInfo light_info;
    std::optional<ShaderShadowInfo> shadow_info;

    engine::Texture irradiancemap;
  };
  PbrShader(const Param& param, Context* context, Object* object);
};

class NormalShader {
 public:
  struct Param {
    bool show_vertex_normal = true;
    bool show_TBN = true;
    bool show_triangle = true;
    bool show_texture_normal = true;
    std::optional<engine::Texture> texture_normal;
    float length = 0.4;
    float width = 1.0;
  };
  NormalShader(const Param& param, Context* context, Object* object);
};

class LinesShader {
 public:
  struct Param {
    float line_width = 1.0;
  };
  LinesShader(const Param& param, Context* context, Object* object);
};

class ColorShader {
 public:
  struct Param {
    glm::vec4 color = glm::vec4(1, 0, 0, 1);
  };
  ColorShader(const Param& param, Context* context, Object* object);
};

class Texture0Shader {
 public:
  struct Param {
    engine::Texture texture0;
  };
  Texture0Shader(const Param& param, Context* context, Object* object);
};

class DepthBufferShader {
 public:
  DepthBufferShader(const engine::Shader& depth_buffer_shader,
                    std::shared_ptr<const engine::Camera> camera, Object* object);
};

class SkyboxShader {
 public:
  struct Param {
    engine::Texture cubemap;
  };
  SkyboxShader(const Param& param, Context* context, Object* object);
};

class FullscreenQuadShader {
 public:
  struct Param {
    engine::Texture texture0;
  };
  FullscreenQuadShader(const Param& param, Context* context, Object* object);
};

class Equirectanglular2CubemapShader {
 public:
  struct Param {
    engine::Texture texture0;
    engine::Camera* camera = nullptr;
  };
  Equirectanglular2CubemapShader(const Param& param, Context* context, Object* object);
};

class TexcoordShader {
 public:
  struct Param {};
  TexcoordShader(const Param& param, Context* context, Object* object);
};

class Cubemap2IrradiancemapShader {
 public:
  struct Param {
    engine::Texture cubemap;
    engine::Camera* camera = nullptr;
  };
  Cubemap2IrradiancemapShader(const Param& param, Context* context, Object* object);
};