#include "gtest/gtest.h"

#include <string>
#include <vector>

#include "renderer/config.h"
#include "renderer/shader_loader.h"

namespace {

TEST(renderer_shader_test, PathTracingSceneShaderParsesWithResolvedIncludes) {
  cg::Config config;
  config.Init(std::string(CG_PROJECT_SOURCE_DIR) + "/playground/config.pb.txt");

  cg::ShaderParser parser("path_tracing_scene");
  const std::vector<cg::ShaderCodePart> code_parts =
      parser.Parse(config.shader_config("path_tracing_scene").cs_path());

  ASSERT_FALSE(code_parts.empty());
  bool found_brdf_include = false;
  for (const cg::ShaderCodePart& code_part : code_parts) {
    if (code_part.glsl_path.ends_with("renderer/shader/bxdf/brdf.glsl")) {
      found_brdf_include = true;
      break;
    }
  }
  EXPECT_TRUE(found_brdf_include);
}

TEST(renderer_shader_test, CubemapAndPbrConfigPathsPointToExistingShaders) {
  cg::Config config;
  config.Init(std::string(CG_PROJECT_SOURCE_DIR) + "/playground/config.pb.txt");

  EXPECT_TRUE(config.shader_config("cubemap_lod").vs_path().ends_with("playground/shader/cubemap_lod.vs.glsl"));
  EXPECT_TRUE(config.shader_config("cubemap_lod").fs_path().ends_with("playground/shader/cubemap_lod.fs.glsl"));
  EXPECT_TRUE(config.shader_config("pbr").vs_path().ends_with("renderer/shader/pbr/pbr.vs.glsl"));
  EXPECT_TRUE(config.shader_config("ray_tracing_canvas")
                  .fs_path()
                  .ends_with("renderer/shader/ray_tracing/ray_tracing_canvas.fs.glsl"));
}

TEST(renderer_shader_test, ShaderLoaderBuildsDescriptorForRenderAndComputeShaders) {
  cg::Config config;
  config.Init(std::string(CG_PROJECT_SOURCE_DIR) + "/playground/config.pb.txt");

  cg::ShaderLoader loader;
  const cg::ShaderProgramDesc render_desc = loader.LoadProgramDesc(
      "sample_scene",
      {{cg::ShaderLoader::kVS, config.shader_config("sample_scene").vs_path()},
       {cg::ShaderLoader::kFS, config.shader_config("sample_scene").fs_path()},
       {cg::ShaderLoader::kGS, ""},
       {cg::ShaderLoader::kTS, ""},
       {cg::ShaderLoader::kCS, ""}});
  EXPECT_EQ(render_desc.name, "sample_scene");
  EXPECT_EQ(render_desc.kind, cg::rhi::ProgramKind::kRender);
  EXPECT_FALSE(render_desc.vs.empty());
  EXPECT_FALSE(render_desc.fs.empty());
  EXPECT_TRUE(render_desc.cs.empty());

  const cg::ShaderProgramDesc compute_desc = loader.LoadProgramDesc(
      "path_tracing_scene",
      {{cg::ShaderLoader::kVS, ""},
       {cg::ShaderLoader::kFS, ""},
       {cg::ShaderLoader::kGS, ""},
       {cg::ShaderLoader::kTS, ""},
       {cg::ShaderLoader::kCS, config.shader_config("path_tracing_scene").cs_path()}});
  EXPECT_EQ(compute_desc.name, "path_tracing_scene");
  EXPECT_EQ(compute_desc.kind, cg::rhi::ProgramKind::kCompute);
  EXPECT_FALSE(compute_desc.cs.empty());
  EXPECT_TRUE(compute_desc.vs.empty());
  EXPECT_TRUE(compute_desc.fs.empty());
}

}  // namespace
