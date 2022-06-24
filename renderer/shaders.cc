#include "renderer/shaders.h"

#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include <set>

#include "renderer/debug.h"
#include "renderer/util.h"

namespace renderer {
PhongShader::PhongShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "phong") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  SetMaterialIndex(object.material_index);
  shader_.SetInt("use_blinn_phong", param.use_blinn_phong);
  Run(scene, object);
}

PbrShader::PbrShader(const Param& pbr, const Scene& scene, const Object& object) 
    : RenderShader(scene, "pbr") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);

  shader_.SetTexture("texture_irradiance_cubemap", pbr.texture_irradiance_cubemap);
  shader_.SetTexture("texture_prefiltered_color_cubemap", pbr.texture_prefiltered_color_cubemap);
  shader_.SetTexture("texture_BRDF_integration_map", pbr.texture_BRDF_integration_map);
}

NormalShader::NormalShader(const Param& param, const Scene& scene, const Object& object) 
    : RenderShader(scene, "normal") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);

  shader_.SetFloat("line_length", param.length_);
  shader_.SetFloat("line_width", param.width_);
  shader_.SetBool("show_vertex_normal", param.show_vertex_normal_);
  shader_.SetBool("show_TBN", param.show_TBN_);
  shader_.SetBool("show_triangle", param.show_triangle_);
}

LinesShader::LinesShader(const Param& param, const Scene& scene, const LinesMesh& lines_mesh,
                         const Transform& transform)
    : RenderShader(scene, "lines") {
  const Camera& camera = scene.camera();
      CGLOGB();
  SetModel(transform.GetModelMatrix());
      CGLOGC();
  SetCamera(camera);
      CGLOGD();
  shader_.SetFloat("line_width", param.line_width);
      CGLOGE() << lines_mesh.positions().size();
  lines_mesh.Submit();
      CGLOGF();
}

ColorShader::ColorShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "color") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  shader_.SetVec4("color", param.color);
  Run(scene, object);
}

TextureShader::TextureShader(const Param& param, const Scene& scene, const Object& object) 
    : RenderShader(scene, "texture") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  shader_.SetTexture("texture0", param.texture0);
}

Texture2DLodShader::Texture2DLodShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "texture2d_lod") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  CGCHECK(param.texture2D0.type() == Texture::Texture2D);
  shader_.SetTexture("texture2D0", param.texture2D0);
}

CubemapLodShader::CubemapLodShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "cubemap_lod") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  CGCHECK(param.cubemap.type() == Texture::Cubemap);
  shader_.SetTexture("texture_cubemap", param.cubemap);
}

/*
DepthBufferShader::DepthBufferShader(const DepthBufferShader::Param& param, const Object& object)
    : RenderShader(param.depth_buffer_shader) {
  SetModel(object);
  SetCamera(camera);
}
*/

CubemapShader::CubemapShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "skybox") {
  SetModel(object);
  SetCamera(scene.camera());
  shader_.SetTexture("texture0", param.cubemap);
}

FullscreenQuadShader::FullscreenQuadShader(const Param& param, const Scene& scene)
    : RenderShader(scene, "fullscreen_quad") {
  shader_.SetTexture("texture0", param.texture0); 
}

PbrEnvironmentCubemapGerneratorShader::PbrEnvironmentCubemapGerneratorShader(const Param& param, const Scene& scene,
                                                                             const Object& object)
    : RenderShader(scene, "equirectangular_2_cubemap_tool") {
  SetModel(object);
  SetCamera(scene.camera());
  shader_.SetTexture("texture2D0", param.texture2D0); 
}

TexcoordShader::TexcoordShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "texcoord") {
  SetModel(object);
  SetCamera(scene.camera());
}

PbrIrradianceCubemapGeneratorShader::PbrIrradianceCubemapGeneratorShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "pbr_irradiance_cubemap_generator") {
  SetModel(object);
  SetCamera(scene.camera());
  shader_.SetTexture("cubemap", param.environment_map);
}

SampleShader::SampleShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "sample_test") {
  SetModel(object);
  SetCamera(scene.camera());
}

PbrPrefilteredColorCubemapGeneratorShader::PbrPrefilteredColorCubemapGeneratorShader(
    const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "pbr_prefiltered_color_cubemap_generator") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  shader_.SetTexture("environment_map", param.environment_map); 
}

PbrBRDFIntegrationMapGeneratorShader::PbrBRDFIntegrationMapGeneratorShader(const Param& param, const Scene& scene)
    : RenderShader(scene, "pbr_BRDF_integration_map_generator") {}

SSAOShader::SSAOShader(const ParamGBuffer& param_g_buffer, const Scene& scene, const Object& object)
    : RenderShader(scene, "SSAO_g_buffer") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
}

SSAOShader::SSAOShader(const ParamSSAO& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "SSAO_SSAO") {
  const Camera& camera = scene.camera();
  shader_.SetMat4("u_projection", camera.GetProjectMatrix());
  shader_.SetTexture("ut_position_vs", param.texture_position_vs);
  shader_.SetTexture("ut_normal_vs", param.texture_normal_vs);
  shader_.SetTexture("ut_noise", param.texture_noise);
  for (int i = 0; i < 64; ++i) {
    shader_.SetVec3(util::Format("u_samples_ts[{}]", i).c_str(), param.sampler_ts[i]);
  }
}

BlurShader::BlurShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "blur") {
  shader_.SetTexture("u_texture_input", param.texture);
  shader_.SetVec2("u_viewport_size", param.viewport_size);
}

RandomShader::RandomShader(const Param& param, const Scene& scene)
    : ComputeShader(scene, "random_test") {
  SetTextures({param.input, param.output});
  SetTextureMetas({{"texture_input", 0, GL_WRITE_ONLY, GL_RGBA32F},
                   {"texture_output", 1, GL_READ_ONLY, GL_RGBA32F}});
  SetWorkGroupNum({param.screen_size.x / 32 + 1, param.screen_size.y / 32 + 1, 1});
  SetTimeSeed(param.frame_num);
  Run();
}

/*
SimpleModelShader::SimpleModelShader(const Scene& scene, ModelObject* model)
    : RenderShader(scene, "simple_model") {
  for (int i = 0; i < model->model_part_num(); ++i) {
    ModelPartObject* model_part = model->mutable_model_part(i);
    SetModel(object);
    SetCamera(camera);
    for (auto& pair : model_part->model_part_data().uniform_2_texture) {
      const std::string uniform_name = pair.first;
      const std::vector<Texture>& textures = pair.second;
      if (textures.size() > 0) {
        std::string use_uniform_name = util::Format("use_{}", uniform_name);
      }
    }
  }
}
*/

/*
InstanceSceneShader::InstanceSceneShader(const Scene& scene, Object* object)
    : RenderShader(scene, "instance_scene") {
  for (int i = 0; i < object->model_part_num(); ++i) {
    ModelPartObject* model_part = object->mutable_model_part(i);
    SetModel(object);
    SetCamera(camera);
    for (auto& pair : model_part->model_part_data().uniform_2_texture) {
      const std::string uniform_name = pair.first;
      const std::vector<Texture>& textures = pair.second;
      if (textures.size() > 0) {
        std::string use_uniform_name = util::Format("use_{}", uniform_name);
      }
    }
  }
}
*/

RayTracingShader::RayTracingShader(const Param& param, const Scene& scene) 
    : ComputeShader(scene, "ray_tracing") {
  SetWorkGroupNum(glm::vec3(param.screen_size.x / 32 + 1, param.screen_size.y / 32 + 1, 1));
  SetCamera(param.camera, true, true);
  SetSpheres(param.spheres);
  SetTextures({param.output});
  SetTextureMetas({{"texture_output", 0, GL_READ_ONLY, GL_RGBA32F}});
  SetScreenSize(param.screen_size);
  Run();
}

PathTracingDemoShader::PathTracingDemoShader(const Param& param, const Scene& scene) 
    : ComputeShader(scene, "path_tracing_demo") {
  SetWorkGroupNum(glm::vec3(param.screen_size.x / 32 + 1, param.screen_size.y / 32 + 1, 1));
  SetCamera(param.camera, true, true);
  SetSpheres(param.spheres);
  SetTextures({param.output});
  SetTextureMetas({{"texture_output", 0, GL_READ_ONLY, GL_RGBA32F}});
  SetScreenSize(param.screen_size);
  SetTimeSeed(param.frame_num);
  Run();
}

RayTracingCanvasShader::RayTracingCanvasShader(const Param& param, const Scene& scene)
    : RenderShader(scene, "ray_tracing_canvas") {
  shader_.SetTexture("texture0", param.texture0); 
  shader_.SetInt("sample_frame_num", param.sample_frame_num); 
}

PathTracingShader::PathTracingShader(const Param& param, const Scene& scene) 
    : ComputeShader(scene, "path_tracing") {
  SetWorkGroupNum(glm::vec3(param.screen_size.x / 32 + 1, param.screen_size.y / 32 + 1, 1));
  SetCamera(param.camera, true, true);
  SetTextures({param.output});
  SetTextureMetas({{"texture_output", 0, GL_READ_ONLY, GL_RGBA32F}});
  SetScreenSize(param.screen_size);
  SetTimeSeed(param.frame_num);
  Run();
}
} // namespace renderer
