#include "renderer/shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "imgui.h"
#include <set>

#include "renderer/bvh.h"
#include "renderer/debug.h"
#include "renderer/mesh/empty_mesh.h"
#include "renderer/geometry.h"
#include "renderer/scene.h"
#include "renderer/util.h"

namespace renderer {
namespace {
void SetCamera(const Camera& camera, ShaderProgram* program) {
  program->SetVec3("camera.pos_ws", camera.transform().translation());
  program->SetMat4("camera.view", camera.GetViewMatrix());
  program->SetMat4("camera.project", camera.GetProjectMatrix());
  program->SetFloat("camera.u_near", camera.near_clip());
  program->SetFloat("camera.u_far", camera.far_clip());
}
void SetCamera1(const Camera& camera_1, ShaderProgram* program) {
  program->SetVec3("camera_1.pos_ws", camera_1.transform().translation());
  program->SetMat4("camera_1.view", camera_1.GetViewMatrix());
  program->SetMat4("camera_1.project", camera_1.GetProjectMatrix());
  program->SetFloat("camera_1.u_near", camera_1.near_clip());
  program->SetFloat("camera_1.u_far", camera_1.far_clip());
}
}

RenderShader::RenderShader(const Scene& scene, const std::string& shader_name) {
  program_ = scene.shader_program_repo().GetShader(shader_name);
  program_.Use();
  if (scene.texture_repo().size() > 0) {
    program_.SetTexture("texture_repo", scene.texture_repo().AsTextureRepo());
  }
  program_.SetInt("light_repo_num", scene.light_repo().num());
  program_.SetInt("material_repo_num", scene.material_repo().num());
  program_.SetInt("bvh_num", scene.bvh().num());
  program_.SetInt("primitive_repo_num", scene.primitive_repo().num());
}

void RenderShader::SetModel(const glm::mat4& model) {
  program_.SetMat4("model", model);
}

void RenderShader::SetModel(const Object& object) {
  program_.SetMat4("model", object.transform.GetModelMatrix());
}

void RenderShader::SetCamera(const Camera& camera) {
  renderer::SetCamera(camera, &program_);
}

void RenderShader::SetCamera1(const Camera& camera_1) {
  renderer::SetCamera1(camera_1, &program_);
}

void RenderShader::SetMaterialIndex(int material_index) {
  program_.SetInt("material_index", material_index);
}

void RenderShader::Run(const Scene& scene, const Object& object) const {
  scene.mesh_repo().GetMesh(object.mesh_index)->Submit();
}

void RenderShader::Run(const Mesh& mesh) const {
  mesh.Submit();
}

ComputeShader::ComputeShader(const Scene& scene, const std::string& shader_name) {
  program_ = scene.shader_program_repo().GetShader(shader_name);
  program_.Use();
  if (scene.texture_repo().size() > 0) {
    program_.SetTexture("texture_repo", scene.texture_repo().AsTextureRepo());
  }
  program_.SetInt("light_repo_num", scene.light_repo().num());
  program_.SetInt("material_repo_num", scene.material_repo().num());
  program_.SetInt("bvh_num", scene.bvh().num());
  program_.SetInt("primitive_repo_num", scene.primitive_repo().num());
}

void ComputeShader::SetWorkGroupNum(const glm::vec3& work_group_num) {
  work_group_num_ = work_group_num;
}

void ComputeShader::SetTextureBinding(const TextureBinding& binding) {
  CheckTextureBindingInternalFormat(binding.texture);
  int texture_unit = program_.SetTexture(binding.uniform_name, binding.texture);
  glBindImageTexture_(texture_unit, binding.texture.id(), 0, GL_FALSE, 0,
                      binding.read_write_type, binding.texture.meta().gl_internal_format);
}

void ComputeShader::CheckTextureBindingInternalFormat(const renderer::Texture& texture) const {
  GLuint internal_format = texture.meta().gl_internal_format;
  std::set<GLint> supported_format = { GL_RGB32F, GL_RGBA32F };
  if (supported_format.count(internal_format) <= 0) {
    CGCHECK(false) << "Unsupported Internal Format : " << std::hex << internal_format << std::dec;
  }
}

void ComputeShader::SetCamera(const Camera& camera) {
  renderer::SetCamera(camera, &program_);
}

void ComputeShader::SetCamera1(const Camera& camera_1) {
  renderer::SetCamera1(camera_1, &program_);
}

void ComputeShader::SetResolution(const glm::vec2& resolution) {
  program_.SetVec2("resolution", resolution);
}

void ComputeShader::SetFrameNum(int frame_num) {
  program_.SetInt("frame_num", frame_num);
}

void ComputeShader::SetDirty(bool dirty) {
  program_.SetBool("dirty", dirty);
}

void ComputeShader::Run() const {
  program_.Use();
  CGCHECK(work_group_num_ != glm::vec3()) << " Setting work group num";
  glDispatchCompute_(work_group_num_.x, work_group_num_.y, work_group_num_.z);
  glMemoryBarrier_(GL_ALL_BARRIER_BITS);
}

PhongShader::PhongShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "phong") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  SetMaterialIndex(object.material_index);
  program_.SetInt("use_blinn_phong", param.use_blinn_phong);
  Run(scene, object);
}

PbrShader::PbrShader(const Param& pbr, const Scene& scene, const Object& object) 
    : RenderShader(scene, "pbr") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);

  program_.SetTexture("texture_irradiance_cubemap", pbr.texture_irradiance_cubemap);
  program_.SetTexture("texture_prefiltered_color_cubemap", pbr.texture_prefiltered_color_cubemap);
  program_.SetTexture("texture_BRDF_integration_map", pbr.texture_BRDF_integration_map);
}

NormalShader::NormalShader(const Param& param, const Scene& scene, const Object& object) 
    : RenderShader(scene, "normal") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);

  program_.SetFloat("line_length", param.length);
  program_.SetFloat("line_width", param.width);
  program_.SetBool("show_triangle", param.show_triangle);
  program_.SetBool("show_face_normal", param.show_face_normal);
  program_.SetBool("show_vertex_texture_normal", param.show_vertex_texture_normal);
  program_.SetBool("show_vertex_normal", param.show_vertex_normal);
  program_.SetBool("show_TBN", param.show_TBN);

  Run(scene, object);
}

LinesShader::LinesShader(const Param& param, const Scene& scene, const LinesMesh& lines_mesh,
                         const Transform& transform)
    : RenderShader(scene, "lines") {
  const Camera& camera = scene.camera();
  SetModel(transform.GetModelMatrix());
  SetCamera(camera);
  program_.SetFloat("line_width", param.line_width);
  lines_mesh.Submit();
}

ColorShader::ColorShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "color") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  program_.SetVec4("color", param.color);
  Run(scene, object);
}

TextureShader::TextureShader(const Param& param, const Scene& scene, const Object& object) 
    : RenderShader(scene, "texture") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  program_.SetTexture("texture0", param.texture0);
}

Texture2DLodShader::Texture2DLodShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "texture2d_lod") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  CGCHECK(param.texture2D0.meta().type == Texture::kTexture2D);
  program_.SetTexture("texture2D0", param.texture2D0);
}

CubemapLodShader::CubemapLodShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "cubemap_lod") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  CGCHECK(param.cubemap.meta().type == Texture::kCubemap);
  program_.SetTexture("texture_cubemap", param.cubemap);
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
  program_.SetTexture("texture0", param.cubemap);
}

FullscreenQuadShader::FullscreenQuadShader(const Param& param, const Scene& scene)
    : RenderShader(scene, "fullscreen_quad") {
  program_.SetTexture("texture0", param.texture0); 
  Run(EmptyMesh());
}

PbrEnvironmentCubemapGerneratorShader::PbrEnvironmentCubemapGerneratorShader(const Param& param, const Scene& scene,
                                                                             const Object& object)
    : RenderShader(scene, "equirectangular_2_cubemap_tool") {
  SetModel(object);
  SetCamera(scene.camera());
  program_.SetTexture("texture2D0", param.texture2D0); 
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
  program_.SetTexture("cubemap", param.environment_map);
}

PbrPrefilteredColorCubemapGeneratorShader::PbrPrefilteredColorCubemapGeneratorShader(
    const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "pbr_prefiltered_color_cubemap_generator") {
  const Camera& camera = scene.camera();
  SetModel(object);
  SetCamera(camera);
  program_.SetTexture("environment_map", param.environment_map); 
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
  program_.SetMat4("u_projection", camera.GetProjectMatrix());
  program_.SetTexture("ut_position_vs", param.texture_position_vs);
  program_.SetTexture("ut_normal_vs", param.texture_normal_vs);
  program_.SetTexture("ut_noise", param.texture_noise);
  for (int i = 0; i < 64; ++i) {
    program_.SetVec3(fmt::format("u_samples_ts[{}]", i).c_str(), param.sampler_ts[i]);
  }
}

BlurShader::BlurShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "blur") {
  program_.SetTexture("u_texture_input", param.texture);
  program_.SetVec2("u_viewport_size", param.viewport_size);
}

RandomShader::RandomShader(const Param& param, const Scene& scene)
    : ComputeShader(scene, "random_test") {
  SetTextureBinding({param.input, "texture_input", GL_WRITE_ONLY});
  SetTextureBinding({param.output, "texture_output", GL_READ_ONLY});
  SetFrameNum(param.frame_num);
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
        std::string use_uniform_name = fmt::format("use_{}", uniform_name);
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
        std::string use_uniform_name = fmt::format("use_{}", uniform_name);
      }
    }
  }
}
*/

RayTracingCanvasShader::RayTracingCanvasShader(const Param& param, const Scene& scene)
    : RenderShader(scene, "ray_tracing_canvas") {
  program_.SetTexture("texture0", param.texture0); 
  program_.SetInt("sample_frame_num", param.sample_frame_num); 
}
} // namespace renderer
