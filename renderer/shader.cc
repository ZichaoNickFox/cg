#include "renderer/shader.h"

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "imgui.h"
#include <set>

#include "base/debug.h"
#include "base/geometry.h"
#include "base/util.h"
#include "renderer/BVH.h"
#include "renderer/mesh/empty_mesh.h"
#include "rhi/device.h"
#include "renderer/scene.h"

namespace cg {
void AppendCameraBindings(const Camera& camera, const std::string& prefix, ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetVec3(util::Format("{}.pos_ws", prefix), camera.transform().translation());
  bindings->SetVec3(util::Format("{}.front", prefix), camera.front_ws());
  bindings->SetMat4(util::Format("{}.view", prefix), camera.GetViewMatrix());
  bindings->SetMat4(util::Format("{}.project", prefix), camera.GetProjectMatrix());
  bindings->SetFloat(util::Format("{}.near", prefix), camera.near_clip());
  bindings->SetFloat(util::Format("{}.far", prefix), camera.far_clip());
}

void AppendModelBindings(const glm::mat4& model, ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetMat4("model", model);
}

void AppendViewBindings(const Camera& camera, ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetMat4("view", camera.GetViewMatrix());
}

void AppendProjectBindings(const Camera& camera, ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetMat4("project", camera.GetProjectMatrix());
}

void AppendViewPosBindings(const glm::vec3& view_pos_ws,
                           ShaderProgramBindings* bindings,
                           const std::string& uniform_name) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetVec3(uniform_name, view_pos_ws);
}

void AppendRenderTransformBindings(const glm::mat4& model,
                                   const Camera& camera,
                                   ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  AppendModelBindings(model, bindings);
  AppendCameraBindings(camera, bindings);
}

void AppendRenderObjectBindings(const Object& object,
                                const Camera& camera,
                                ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  AppendRenderTransformBindings(object.transform.GetModelMatrix(), camera, bindings);
}

void AppendLegacyRenderTransformBindings(const glm::mat4& model,
                                         const Camera& camera,
                                         ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  AppendModelBindings(model, bindings);
  AppendViewBindings(camera, bindings);
  AppendProjectBindings(camera, bindings);
}

void AppendLegacyRenderObjectBindings(const Object& object,
                                      const Camera& camera,
                                      ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  AppendLegacyRenderTransformBindings(object.transform.GetModelMatrix(), camera, bindings);
}

void AppendMaterialIndexBindings(int material_index, ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetInt("material_index", material_index);
}

void AppendPrimitiveStartIndexBindings(int primitive_start_index, ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetInt("primitive_start_index", primitive_start_index);
}

void AppendResolutionBindings(const glm::vec2& resolution, ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetVec2("resolution", resolution);
}

void AppendFrameNumBindings(int frame_num, ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetInt("frame_num", frame_num);
}

void AppendDirtyBindings(bool dirty, ShaderProgramBindings* bindings) {
  CGCHECK_NOTNULL(bindings);
  bindings->SetBool("dirty", dirty);
}

namespace {
const Texture& TextureRepoFallbackTextureArray() {
  static const Texture texture = [] {
    Texture::Meta meta;
    meta.type = Texture::kTexture2DArray;
    meta.width = 1;
    meta.height = 1;
    meta.channel_num = 4;
    meta.hdr = false;
    meta.level_num = 1;
    meta.depth = 1;
    meta.format = rhi::TextureFormat::kRGBA8;
    meta.pixel_format = rhi::PixelFormat::kRGBA;
    meta.pixel_type = rhi::PixelType::kUInt8;
    meta.min_filter = rhi::FilterMode::kNearest;
    meta.mag_filter = rhi::FilterMode::kNearest;
    meta.wrap_s = rhi::WrapMode::kClampToEdge;
    meta.wrap_t = rhi::WrapMode::kClampToEdge;

    const std::array<uint8_t, 4> white = {255, 255, 255, 255};
    Texture fallback(meta);
    fallback.SetCpuLevelData(0, white.data(), white.size());
    fallback.Varify();
    return fallback;
  }();
  return texture;
}

ShaderProgramBindings BuildShaderCommonBindings(const Scene& scene) {
  ShaderProgramBindings bindings;
  bindings.SetBufferBinding(scene.light_repo().binding_desc());
  bindings.SetBufferBinding(scene.material_repo().binding_desc());
  bindings.SetBufferBinding(scene.bvh().binding_desc());
  bindings.SetBufferBinding(scene.primitive_repo().binding_desc());
  if (scene.texture_repo().size() > 0) {
    bindings.SetTexture("texture_repo", scene.texture_repo().AsTexture2DArray());
  } else {
    bindings.SetTexture("texture_repo", TextureRepoFallbackTextureArray());
  }
  bindings.SetInt("light_repo_num", scene.light_repo().num());
  bindings.SetInt("material_repo_num", scene.material_repo().num());
  bindings.SetInt("bvh_num", scene.bvh().num());
  bindings.SetInt("primitive_repo_num", scene.primitive_repo().num());
  bindings.SetInt("primitive_light_num", scene.light_repo().primitive_light_num());
  bindings.SetFloat("primitive_light_area", scene.light_repo().primitive_light_area());
  return bindings;
}

ShaderProgramBindings BuildMergedBindings(const ShaderProgramBindings& common,
                                         const ShaderProgramBindings& extra) {
  ShaderProgramBindings merged = common;
  merged.Append(extra);
  return merged;
}
}

RenderShader::RenderShader(const Scene& scene, const std::string& shader_name) {
  program_ = scene.shader_program_repo().GetShader(shader_name);
  common_bindings_ = BuildShaderCommonBindings(scene);
}

void RenderShader::ApplyBindings(const ShaderProgramBindings& bindings) const {
  program_.ApplyBindings(BuildMergedBindings(common_bindings_, bindings));
}

void RenderShader::DrawBindings(const ShaderProgramBindings& bindings,
                                const Scene& scene,
                                const Object& object) const {
  DrawBindings(bindings, *scene.mesh_repo().GetMesh(object.mesh_index));
}

void RenderShader::DrawBindings(const ShaderProgramBindings& bindings,
                                const Mesh& mesh) const {
  program_.DrawBindings(BuildMergedBindings(common_bindings_, bindings), mesh.BuildDrawDesc());
}

ComputeShader::ComputeShader(const Scene& scene, const std::string& shader_name) {
  program_ = scene.shader_program_repo().GetShader(shader_name);
  common_bindings_ = BuildShaderCommonBindings(scene);
}

void ComputeShader::DispatchBindings(const ShaderProgramBindings& bindings,
                                     const rhi::ComputeDispatchDesc& desc) const {
  program_.DispatchComputeBindings(BuildMergedBindings(common_bindings_, bindings), desc);
}

PhongShader::PhongShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "phong") {
  ShaderProgramBindings bindings;
  AppendRenderObjectBindings(object, scene.camera(), &bindings);
  AppendMaterialIndexBindings(object.material_index, &bindings);
  bindings.SetBool("use_blinn_phong", param.use_blinn_phong);
  DrawBindings(bindings, scene, object);
}

PbrShader::PbrShader(const Param& pbr, const Scene& scene, const Object& object) 
    : RenderShader(scene, "pbr") {
  ShaderProgramBindings bindings;
  AppendLegacyRenderObjectBindings(object, scene.camera(), &bindings);
  AppendViewPosBindings(scene.camera(), &bindings);
  bindings.SetTexture("texture_irradiance_cubemap", pbr.texture_irradiance_cubemap);
  bindings.SetTexture("texture_prefiltered_color_cubemap", pbr.texture_prefiltered_color_cubemap);
  bindings.SetTexture("texture_BRDF_integration_map", pbr.texture_BRDF_integration_map);
  ApplyBindings(bindings);
}

NormalShader::NormalShader(const Param& param, const Scene& scene, const Object& object) 
    : RenderShader(scene, "normal") {
  ShaderProgramBindings bindings;
  AppendRenderObjectBindings(object, scene.camera(), &bindings);
  bindings.SetFloat("line_length", param.length);
  bindings.SetFloat("line_width", param.width);
  bindings.SetBool("show_triangle", param.show_triangle);
  bindings.SetBool("show_face_normal", param.show_face_normal);
  bindings.SetBool("show_vertex_texture_normal", param.show_vertex_texture_normal);
  bindings.SetBool("show_vertex_normal", param.show_vertex_normal);
  bindings.SetBool("show_TBN", param.show_TBN);
  DrawBindings(bindings, scene, object);
}

LinesShader::LinesShader(const Param& param, const Scene& scene, const LinesMesh& lines_mesh,
                         const Transform& transform)
    : RenderShader(scene, "lines") {
  ShaderProgramBindings bindings;
  AppendRenderTransformBindings(transform.GetModelMatrix(), scene.camera(), &bindings);
  bindings.SetFloat("line_width", param.line_width);
  DrawBindings(bindings, lines_mesh);
}

ColorShader::ColorShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "color") {
  ShaderProgramBindings bindings;
  AppendRenderObjectBindings(object, scene.camera(), &bindings);
  bindings.SetVec4("color", param.color);
  DrawBindings(bindings, scene, object);
}

TextureShader::TextureShader(const Param& param, const Scene& scene, const Object& object) 
    : RenderShader(scene, "texture") {
  ShaderProgramBindings bindings;
  AppendLegacyRenderObjectBindings(object, scene.camera(), &bindings);
  bindings.SetTexture("texture0", param.texture0);
  DrawBindings(bindings, scene, object);
}

Texture2DLodShader::Texture2DLodShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "texture2d_lod") {
  CGCHECK(param.texture2D0.meta().type == Texture::kTexture2D);
  ShaderProgramBindings bindings;
  AppendLegacyRenderObjectBindings(object, scene.camera(), &bindings);
  AppendViewPosBindings(param.view_pos_ws, &bindings);
  bindings.SetTexture("texture2D0", param.texture2D0);
  DrawBindings(bindings, scene, object);
}

CubemapLodShader::CubemapLodShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "cubemap_lod") {
  CGCHECK(param.cubemap.meta().type == Texture::kCubemap);
  ShaderProgramBindings bindings;
  AppendLegacyRenderObjectBindings(object, scene.camera(), &bindings);
  AppendViewPosBindings(param.view_pos_ws, &bindings);
  bindings.SetTexture("texture_cubemap", param.cubemap);
  DrawBindings(bindings, scene, object);
}

CubemapShader::CubemapShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "skybox") {
  ShaderProgramBindings bindings;
  AppendLegacyRenderObjectBindings(object, scene.camera(), &bindings);
  bindings.SetTexture("texture0", param.cubemap);
  DrawBindings(bindings, scene, object);
}

FullscreenQuadShader::FullscreenQuadShader(const Param& param, const Scene& scene)
    : RenderShader(scene, "fullscreen_quad") {
  ShaderProgramBindings bindings;
  bindings.SetTexture("texture0", param.texture0);
  DrawBindings(bindings, EmptyMesh());
}

PbrEnvironmentCubemapGerneratorShader::PbrEnvironmentCubemapGerneratorShader(const Param& param, const Scene& scene,
                                                                             const Object& object)
    : RenderShader(scene, "equirectangular_2_cubemap_tool") {
  ShaderProgramBindings bindings;
  AppendLegacyRenderObjectBindings(object, scene.camera(), &bindings);
  bindings.SetTexture("texture2D0", param.texture2D0);
  DrawBindings(bindings, scene, object);
}

TexcoordShader::TexcoordShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "texcoord") {
  (void)param;
  ShaderProgramBindings bindings;
  AppendLegacyRenderObjectBindings(object, scene.camera(), &bindings);
  DrawBindings(bindings, scene, object);
}

PbrIrradianceCubemapGeneratorShader::PbrIrradianceCubemapGeneratorShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "pbr_irradiance_cubemap_generator") {
  ShaderProgramBindings bindings;
  AppendLegacyRenderObjectBindings(object, scene.camera(), &bindings);
  bindings.SetTexture("cubemap", param.environment_map);
  DrawBindings(bindings, scene, object);
}

PbrPrefilteredColorCubemapGeneratorShader::PbrPrefilteredColorCubemapGeneratorShader(
    const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "pbr_prefiltered_color_cubemap_generator") {
  ShaderProgramBindings bindings;
  AppendLegacyRenderObjectBindings(object, scene.camera(), &bindings);
  bindings.SetTexture("environment_map", param.environment_map);
  bindings.SetFloat("roughness", param.roughness);
  DrawBindings(bindings, scene, object);
}

PbrBRDFIntegrationMapGeneratorShader::PbrBRDFIntegrationMapGeneratorShader(const Param& param, const Scene& scene)
    : RenderShader(scene, "pbr_BRDF_integration_map_generator") {
  (void)param;
  DrawBindings({}, EmptyMesh());
}

BlurShader::BlurShader(const Param& param, const Scene& scene, const Object& object)
    : RenderShader(scene, "blur") {
  (void)object;
  ShaderProgramBindings bindings;
  bindings.SetTexture("u_texture_input", param.texture);
  bindings.SetVec2("u_viewport_size", param.viewport_size);
  DrawBindings(bindings, EmptyMesh());
}

RandomShader::RandomShader(const Param& param, const Scene& scene)
    : ComputeShader(scene, "random_test") {
  ShaderProgramBindings bindings;
  bindings.SetStorageTexture("texture_input", param.input, TextureAccess::kWriteOnly);
  bindings.SetStorageTexture("texture_output", param.output, TextureAccess::kReadOnly);
  AppendFrameNumBindings(scene.frame_stat().frame_num(), &bindings);
  DispatchBindings(bindings, {
      .workgroup_count = glm::uvec3(param.screen_size.x, param.screen_size.y, 1),
  });
}

RayTracingCanvasShader::RayTracingCanvasShader(const Param& param, const Scene& scene)
    : RenderShader(scene, "ray_tracing_canvas") {
  ShaderProgramBindings bindings;
  bindings.SetTexture("texture0", param.texture0);
  bindings.SetInt("sample_frame_num", param.sample_frame_num);
  DrawBindings(bindings, EmptyMesh());
}
} // namespace cg
