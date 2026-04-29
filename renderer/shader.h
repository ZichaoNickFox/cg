#pragma once

#include <glm/glm.hpp>

#include "base/geometry.h"
#include "renderer/camera.h"
#include "renderer/mesh/lines_mesh.h"
#include "renderer/object.h"
#include "rhi/device.h"
#include "renderer/shader_program.h"
#include "renderer/texture.h"

namespace cg {

using TextureAccess = rhi::TextureAccess;

class Scene;
void AppendCameraBindings(const Camera& camera,
                          const std::string& prefix,
                          ShaderProgramBindings* bindings);
inline void AppendCameraBindings(const Camera& camera, ShaderProgramBindings* bindings) {
  AppendCameraBindings(camera, "camera", bindings);
}
void AppendModelBindings(const glm::mat4& model, ShaderProgramBindings* bindings);
void AppendViewBindings(const Camera& camera, ShaderProgramBindings* bindings);
void AppendProjectBindings(const Camera& camera, ShaderProgramBindings* bindings);
void AppendViewPosBindings(const glm::vec3& view_pos_ws,
                           ShaderProgramBindings* bindings,
                           const std::string& uniform_name = "view_pos_ws");
inline void AppendViewPosBindings(const Camera& camera,
                                  ShaderProgramBindings* bindings,
                                  const std::string& uniform_name = "view_pos_ws") {
  AppendViewPosBindings(camera.transform().translation(), bindings, uniform_name);
}
void AppendRenderTransformBindings(const glm::mat4& model,
                                   const Camera& camera,
                                   ShaderProgramBindings* bindings);
void AppendRenderObjectBindings(const Object& object,
                                const Camera& camera,
                                ShaderProgramBindings* bindings);
void AppendLegacyRenderTransformBindings(const glm::mat4& model,
                                         const Camera& camera,
                                         ShaderProgramBindings* bindings);
void AppendLegacyRenderObjectBindings(const Object& object,
                                      const Camera& camera,
                                      ShaderProgramBindings* bindings);
void AppendMaterialIndexBindings(int material_index, ShaderProgramBindings* bindings);
void AppendPrimitiveStartIndexBindings(int primitive_start_index, ShaderProgramBindings* bindings);
void AppendResolutionBindings(const glm::vec2& resolution, ShaderProgramBindings* bindings);
void AppendFrameNumBindings(int frame_num, ShaderProgramBindings* bindings);
void AppendDirtyBindings(bool dirty, ShaderProgramBindings* bindings);

class ComputeShader {
 protected:
  ComputeShader(const Scene& scene, const std::string& shader_name);

  void DispatchBindings(const ShaderProgramBindings& bindings,
                        const rhi::ComputeDispatchDesc& desc) const;

  ShaderProgramBindings common_bindings_;
  ShaderProgram program_;
};

class RenderShader {
 protected:
  RenderShader(const Scene& scene, const std::string& shader_name);
  void ApplyBindings(const ShaderProgramBindings& bindings) const;
  void DrawBindings(const ShaderProgramBindings& bindings,
                    const Scene& scene,
                    const Object& object) const;
  void DrawBindings(const ShaderProgramBindings& bindings,
                    const Mesh& mesh) const;

 protected:
  ShaderProgramBindings common_bindings_;
  ShaderProgram program_;
};

class PhongShader : public cg::RenderShader {
 public:
  struct Param {
    bool use_blinn_phong = true;
  };
  PhongShader(const Param& param, const Scene& scene, const Object& object);
};

class PbrShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture texture_irradiance_cubemap;
    cg::Texture texture_prefiltered_color_cubemap;
    cg::Texture texture_BRDF_integration_map;
  };
  PbrShader(const Param& param, const Scene& scene, const Object& object);
};

class NormalShader : public cg::RenderShader {
 public:
  struct Param {
    bool show_triangle = false;
    bool show_face_normal = false;
    bool show_vertex_normal = false;
    bool show_vertex_texture_normal = false;
    bool show_TBN = false;
    float length = 0.1;
    float width = 3;
  };
  NormalShader(const Param& param, const Scene& scene, const Object& object);
};

class LinesShader : public cg::RenderShader {
 public:
  struct Param {
    float line_width = 1.0;
  };
  LinesShader(const Param& param, const Scene& scene, const LinesMesh& line_mesh,
              const Transform& transform = Transform());
};

class ColorShader : public cg::RenderShader {
 public:
  struct Param {
    glm::vec4 color = glm::vec4(1, 0, 0, 1);
  };
  ColorShader(const Param& param, const Scene& scene, const Object& object);
};

class TextureShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture texture0;
  };
  TextureShader(const Param& param, const Scene& scene, const Object& object);
};

class Texture2DLodShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture texture2D0;
    glm::vec3 view_pos_ws = glm::vec3(0.0f);
  };
  Texture2DLodShader(const Param& param, const Scene& scene, const Object& object);
};

class CubemapLodShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture cubemap;
    glm::vec3 view_pos_ws;
  };
  CubemapLodShader(const Param& param, const Scene& scene, const Object& object);
};

/*
class DepthBufferShader : public cg::RenderShader {
 public:
  struct Param {
    const cg::Camera* camera;
    cg::Shader depth_buffer_shader;
  };
  DepthBufferShader(const Param& param, const Object& object);
};
*/

class CubemapShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture cubemap;
  };
  CubemapShader(const Param& param, const Scene& scene, const Object& object);
};

class FullscreenQuadShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture texture0;
  };
  FullscreenQuadShader(const Param& param, const Scene& scene);
};

class PbrEnvironmentCubemapGerneratorShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture texture2D0;
    cg::Camera* camera = nullptr;
  };
  PbrEnvironmentCubemapGerneratorShader(const Param& param, const Scene& scene, const Object& object);
};

class TexcoordShader : public cg::RenderShader {
 public:
  struct Param {};
  TexcoordShader(const Param& param, const Scene& scene, const Object& object);
};

class PbrIrradianceCubemapGeneratorShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture environment_map;
    cg::Camera* camera = nullptr;
  };
  PbrIrradianceCubemapGeneratorShader(const Param& param, const Scene& scene, const Object& object);
};

class PbrPrefilteredColorCubemapGeneratorShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture environment_map;
    cg::Camera* camera = nullptr;
    float roughness = 0.0f;
  };
  PbrPrefilteredColorCubemapGeneratorShader(const Param& param, const Scene& scene, const Object& object);
};

class PbrBRDFIntegrationMapGeneratorShader : public cg::RenderShader {
 public:
  struct Param {};
  PbrBRDFIntegrationMapGeneratorShader(const Param& param, const Scene& scene);
};

class BlurShader : public cg::RenderShader {
 public:
  struct Param{
    cg::Texture texture;
    glm::vec2 viewport_size;
  };
  BlurShader(const Param& param, const Scene& scene, const Object& object);
};

class RandomShader : public cg::ComputeShader {
 public:
  struct Param{
    glm::ivec2 screen_size;
    cg::Texture input;
    cg::Texture output;
    int frame_num;
  };
  RandomShader(const Param& param, const Scene& scene);

 private:
  Param param_;
};

class RayTracingCanvasShader : public cg::RenderShader {
 public:
  struct Param {
    cg::Texture texture0;
    int sample_frame_num = 1;
  };
  RayTracingCanvasShader(const Param& param, const Scene& scene);
};
} // namespace cg
