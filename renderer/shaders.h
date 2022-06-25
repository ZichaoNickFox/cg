#pragma once

#include <optional>
#include <glm/glm.hpp>

#include "renderer/camera.h"
#include "renderer/compute_shader.h"
#include "renderer/scene.h"
#include "renderer/material.h"
#include "renderer/meshes/lines_mesh.h"
#include "renderer/object.h"
#include "renderer/pass.h"
#include "renderer/render_shader.h"
#include "renderer/shader.h"
#include "renderer/texture.h"

namespace renderer {
class PhongShader : public renderer::RenderShader {
 public:
  struct Param {
    bool use_blinn_phong = true;
  };
  PhongShader(const Param& param, const Scene& scene, const Object& object);
};

class PbrShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture texture_irradiance_cubemap;
    renderer::Texture texture_prefiltered_color_cubemap;
    renderer::Texture texture_BRDF_integration_map;
  };
  PbrShader(const Param& param, const Scene& scene, const Object& object);
};

class NormalShader : public renderer::RenderShader {
 public:
  struct Param {
    bool show_vertex_normal_;
    bool show_TBN_;
    bool show_triangle_;
    bool show_texture_normal_;
    float length_;
    float width_;
  };
  NormalShader(const Param& param, const Scene& scene, const Object& object);
};

class LinesShader : public renderer::RenderShader {
 public:
  struct Param {
    float line_width = 1.0;
  };
  LinesShader(const Param& param, const Scene& scene, const LinesMesh& line_mesh,
              const Transform& transform = Transform());
};

class ColorShader : public renderer::RenderShader {
 public:
  struct Param {
    glm::vec4 color = glm::vec4(1, 0, 0, 1);
  };
  ColorShader(const Param& param, const Scene& scene, const Object& object);
};

class TextureShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture texture0;
  };
  TextureShader(const Param& param, const Scene& scene, const Object& object);
};

class Texture2DLodShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture texture2D0;
  };
  Texture2DLodShader(const Param& param, const Scene& scene, const Object& object);
};

class CubemapLodShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture cubemap;
    glm::vec3 view_pos_ws;
  };
  CubemapLodShader(const Param& param, const Scene& scene, const Object& object);
};

/*
class DepthBufferShader : public renderer::RenderShader {
 public:
  struct Param {
    const renderer::Camera* camera;
    renderer::Shader depth_buffer_shader;
  };
  DepthBufferShader(const Param& param, const Object& object);
};
*/

class CubemapShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture cubemap;
  };
  CubemapShader(const Param& param, const Scene& scene, const Object& object);
};

class FullscreenQuadShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture texture0;
  };
  FullscreenQuadShader(const Param& param, const Scene& scene);
};

class PbrEnvironmentCubemapGerneratorShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture texture2D0;
    renderer::Camera* camera = nullptr;
  };
  PbrEnvironmentCubemapGerneratorShader(const Param& param, const Scene& scene, const Object& object);
};

class TexcoordShader : public renderer::RenderShader {
 public:
  struct Param {};
  TexcoordShader(const Param& param, const Scene& scene, const Object& object);
};

class PbrIrradianceCubemapGeneratorShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture environment_map;
    renderer::Camera* camera = nullptr;
  };
  PbrIrradianceCubemapGeneratorShader(const Param& param, const Scene& scene, const Object& object);
};

class SampleShader : public renderer::RenderShader {
 public:
  struct Param {
  };
  SampleShader(const Param& param, const Scene& scene, const Object& object);
};

class PbrPrefilteredColorCubemapGeneratorShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture environment_map;
    renderer::Camera* camera = nullptr;
  };
  PbrPrefilteredColorCubemapGeneratorShader(const Param& param, const Scene& scene, const Object& object);
};

class PbrBRDFIntegrationMapGeneratorShader : public renderer::RenderShader {
 public:
  struct Param {};
  PbrBRDFIntegrationMapGeneratorShader(const Param& param, const Scene& scene);
};

class SSAOShader : public renderer::RenderShader {
 public:
  struct ParamGBuffer {};
  struct ParamSSAO {
    renderer::Texture texture_position_vs;
    renderer::Texture texture_normal_vs;
    renderer::Texture texture_noise;
    std::array<glm::vec3, 64> sampler_ts;
  };
  SSAOShader(const ParamGBuffer& param, const Scene& scene, const Object& object);
  SSAOShader(const ParamSSAO& param, const Scene& scene, const Object& object);
};

class BlurShader : public renderer::RenderShader {
 public:
  struct Param{
    renderer::Texture texture;
    glm::vec2 viewport_size;
  };
  BlurShader(const Param& param, const Scene& scene, const Object& object);
};

class RandomShader : public renderer::ComputeShader {
 public:
  struct Param{
    glm::ivec2 screen_size;
    renderer::Texture input;
    renderer::Texture output;
    int frame_num;
  };
  RandomShader(const Param& param, const Scene& scene);

 private:
  Param param_;
};

class RayTracingShader : public renderer::ComputeShader {
 public:
  struct Param {
    glm::ivec2 screen_size;
    renderer::Camera* camera = nullptr;
    std::vector<renderer::Sphere> spheres;
    renderer::Texture output;
  };
  RayTracingShader(const Param& param, const Scene& scene);
};

class PathTracingDemoShader : public renderer::ComputeShader {
 public:
  struct Param {
    glm::ivec2 screen_size;
    renderer::Camera* camera = nullptr;
    std::vector<renderer::Sphere> spheres;
    int frame_num;
    renderer::Texture output;
  };
  PathTracingDemoShader(const Param& param, const Scene& scene);
};

class RayTracingCanvasShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture texture0;
    int sample_frame_num = 1;
  };
  RayTracingCanvasShader(const Param& param, const Scene& scene);
};

class PathTracingShader : public renderer::ComputeShader {
 public:
  struct Param {
    glm::ivec2 screen_size;
    renderer::Camera* camera = nullptr;
    int frame_num;
    renderer::Texture output;
  };
  PathTracingShader(const Param& param, const Scene& scene);
};
} // namespace renderer