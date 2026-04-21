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
  const std::vector<cg::ShaderProgram::CodePart> code_parts =
      parser.Parse(config.shader_config("path_tracing_scene").cs_path());

  ASSERT_FALSE(code_parts.empty());
  bool found_brdf_include = false;
  for (const cg::ShaderProgram::CodePart& code_part : code_parts) {
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

}  // namespace
