#pragma once

#include <optional>
#include <glm/glm.hpp>

#include "renderer/camera.h"
#include "renderer/geometry.h"
#include "renderer/mesh/lines_mesh.h"
#include "renderer/object.h"
#include "renderer/shader.h"
#include "renderer/shader_program.h"
#include "renderer/texture.h"

namespace renderer {

class Scene;
class ComputeShader {
 public:
  struct TextureBinding {
    Texture texture;
    std::string uniform_name;
    GLuint read_write_type = GL_READ_ONLY;
  };
  void Run() const;

 protected:
  ComputeShader(const Scene& scene, const std::string& shader_name);

  void SetTextureBinding(const TextureBinding& texture_binding);

  void SetCamera(const Camera& camera);
  void SetCamera1(const Camera& camera_1);
  void SetResolution(const glm::vec2& resolution);
  void SetFrameNum(const Scene& scene);
  void SetDirty(bool dirty);
  void SetWorkGroupNum(const glm::vec3& work_group_num);

 protected:
  void CheckTextureBindingInternalFormat(const renderer::Texture& texture) const;

  ShaderProgram program_;
  glm::vec3 work_group_num_;
};

class RenderShader {
 protected:
  RenderShader(const Scene& scene, const std::string& shader_name);
  void Run(const Scene& scene, const Object& object) const;
  void Run(const Mesh& mesh) const;

 public:
  void SetModel(const glm::mat4& model);
  void SetModel(const Object& object);
  void SetCamera(const Camera& camera);
  void SetCamera1(const Camera& camera_1);
  void SetMaterialIndex(int material_index);
  void SetNearFar(const Camera& camera);
  void SetScreenSize(const glm::vec2& screen_size);
  void SetTimeSeed(int frame_num);

 protected:
  ShaderProgram program_;
};

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

class RayTracingCanvasShader : public renderer::RenderShader {
 public:
  struct Param {
    renderer::Texture texture0;
    int sample_frame_num = 1;
  };
  RayTracingCanvasShader(const Param& param, const Scene& scene);
};
} // namespace renderer