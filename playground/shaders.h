#pragma once

#include <optional>
#include <glm/glm.hpp>

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/empty_object.h"
#include "playground/object/object.h"
#include "playground/object/point_light.h"

class ShaderShadowInfo {
 public:
  ShaderShadowInfo() {}
  ShaderShadowInfo(const glm::mat4& light_space_vp, engine::Texture texture_depth);

  void UpdateMaterial(Context* context, engine::Material* material) const;

 private:
  glm::mat4 light_space_vp_;
  engine::Texture texture_depth_;
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
    Param(const glm::vec3& ambient = glm::vec3(0.1, 0, 0), const glm::vec3& diffuse = glm::vec3(1, 0, 0),
          const glm::vec3& specular = glm::vec3(1, 1, 1), float shininess = 5, bool use_blinn_phong = true)
        : ambient_(ambient), diffuse_(diffuse), specular_(specular), shininess_(shininess),
          use_blinn_phong_(use_blinn_phong) {}
   private:
    void Gui();
    glm::vec3 ambient_;
    glm::vec3 diffuse_;
    glm::vec3 specular_;
    float shininess_;
    bool use_blinn_phong_;

   public:
    std::optional<engine::Texture> texture_ambient;
    std::optional<engine::Texture> texture_normal;
    std::optional<engine::Texture> texture_specular;
    std::optional<engine::Texture> texture_diffuse;

    ShaderLightInfo light_info;
    std::optional<ShaderShadowInfo> shadow_info;
  
   friend class PhongShader;
  };
  PhongShader(Param* param, Context* context, Object* object);
};

class PbrShader {
 public:
  struct Param {
    Param(const glm::vec3& albedo = glm::vec3(1, 0, 0), float metallic = 0.5, float roughness = 0.5,
          glm::vec3 ao = glm::vec3(1.0, 1.0, 1.0))
        : albedo_(albedo), metallic_(metallic), roughness_(roughness), ao_(ao) {}
    void Gui(); 

   private:
    glm::vec3 albedo_;
    float metallic_;
    float roughness_;
    glm::vec3 ao_;

   public:
    std::optional<engine::Texture> texture_normal;
    std::optional<engine::Texture> texture_albedo;
    std::optional<engine::Texture> texture_metallic;
    std::optional<engine::Texture> texture_roughness;
    std::optional<engine::Texture> texture_ao;
    engine::Texture texture_irradiance_cubemap;
    engine::Texture texture_prefiltered_color_cubemap;
    engine::Texture texture_BRDF_integration_map;

    ShaderLightInfo light_info;
    std::optional<ShaderShadowInfo> shadow_info;

    friend class PbrShader;
  };
  PbrShader(Param* param, Context* context, Object* object);
};

class NormalShader {
 public:
  struct Param {
    Param(bool show_vertex_normal = true, bool show_TBN = true, bool show_triangle = true,
          float show_texture_normal = true, float length = 0.4, float width = 1.0,
          std::optional<engine::Texture> in_texture_normal = std::optional<engine::Texture>())
        : show_vertex_normal_(show_vertex_normal), show_TBN_(show_TBN), show_triangle_(show_triangle),
          show_texture_normal_(show_texture_normal), length_(length), width_(width),
          texture_normal(in_texture_normal) {}
   private:
      void Gui();

      bool show_vertex_normal_ = true;
      bool show_TBN_ = true;
      bool show_triangle_ = true;
      bool show_texture_normal_ = true;
      float length_ = 0.4;
      float width_ = 1.0;
   public:
    std::optional<engine::Texture> texture_normal;

    friend class NormalShader;
  };
  NormalShader(Param* param, Context* context, Object* object);
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

class TextureShader {
 public:
  struct Param {
    engine::Texture texture0;
  };
  TextureShader(const Param& param, Context* context, Object* object);
};

class Texture2DLodShader {
 public:
  struct Param {
    engine::Texture texture2D0;
    glm::vec3 view_pos_ws;
  };
  Texture2DLodShader(const Param& param, Context* context, Object* object);
};

class CubemapLodShader {
 public:
  struct Param {
    engine::Texture cubemap;
    glm::vec3 view_pos_ws;
  };
  CubemapLodShader(const Param& param, Context* context, Object* object);
};

class DepthBufferShader {
 public:
  DepthBufferShader(const engine::Shader& depth_buffer_shader,
                    std::shared_ptr<const engine::Camera> camera, Object* object);
};

class CubemapShader {
 public:
  struct Param {
    engine::Texture cubemap;
  };
  CubemapShader(const Param& param, Context* context, Object* object);
};

class FullscreenQuadShader {
 public:
  struct Param {
    engine::Texture texture0;
  };
  FullscreenQuadShader(const Param& param, Context* context, EmptyObject* empty_object);
};

class PbrEnvironmentCubemapGerneratorShader {
 public:
  struct Param {
    engine::Texture texture2D0;
    engine::Camera* camera = nullptr;
  };
  PbrEnvironmentCubemapGerneratorShader(const Param& param, Context* context, Object* object);
};

class TexcoordShader {
 public:
  struct Param {};
  TexcoordShader(const Param& param, Context* context, Object* object);
};

class PbrIrradianceCubemapGeneratorShader {
 public:
  struct Param {
    engine::Texture environment_map;
    engine::Camera* camera = nullptr;
  };
  PbrIrradianceCubemapGeneratorShader(const Param& param, Context* context, Object* object);
};

class SampleShader {
 public:
  struct Param {
  };
  SampleShader(const Param& param, Context* context, Object* object);
};

class PbrPrefilteredColorCubemapGeneratorShader {
 public:
  struct Param {
    engine::Texture environment_map;
    engine::Camera* camera = nullptr;
    float roughness = 0.1;
  };
  PbrPrefilteredColorCubemapGeneratorShader(const Param& param, Context* context, Object* object);
};

class PbrBRDFIntegrationMapGeneratorShader {
 public:
  struct Param {};
  PbrBRDFIntegrationMapGeneratorShader(const Param& param, Context* context, EmptyObject* empty_object);
};