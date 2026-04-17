#include "gtest/gtest.h"

#include <glm/glm.hpp>
#include <map>
#include <math.h>

#include "renderer/config.h"
#include "base/math.h"
#include "base/util.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"

TEST(renderer_test, util)
{
  EXPECT_EQ(util::FileExt("a.txt"), "txt");
  EXPECT_EQ(util::FileExt("a.fs.glsl"), "glsl");
  EXPECT_EQ(util::FileExt("a"), "");

  EXPECT_EQ(util::FileName("a/target.png"), "target");
  EXPECT_EQ(util::FileName("target.png"), "target");
  EXPECT_EQ(util::FileName("target"), "target");
  EXPECT_EQ(util::FileName("a/target"), "target");

  EXPECT_TRUE(util::StartsWith("abc", "a"));
  EXPECT_TRUE(util::StartsWith("abc", "ab"));
  EXPECT_TRUE(util::StartsWith("abc", "abc"));
  EXPECT_TRUE(util::EndsWith("abc", "c"));
  EXPECT_TRUE(util::EndsWith("abc", "bc"));
  EXPECT_TRUE(util::EndsWith("abc", "abc"));
  EXPECT_TRUE(util::EndsWith("abc", "c"));

  EXPECT_EQ(util::FileDir("abc.png"), "");
  EXPECT_EQ(util::FileDir("/abc.png"), "");
  EXPECT_EQ(util::FileDir("a/abc.png"), "a");

  EXPECT_EQ(util::ReplaceBackslash("a\\abc.png"), "a/abc.png");
  EXPECT_EQ(util::ReplaceBackslash("\\"), "/");
  EXPECT_EQ(util::ReplaceBackslash("a/abc.png"), "a/abc.png");

  glm::vec3 array[] = {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}};
  EXPECT_EQ(util::AsVector(array).size(), 3);

  std::vector<int> vs = {1, 2, 3, 4, 4, 3, 2, 1, 2, 3, 4};
  util::Remove(3, &vs);
  std::vector<int> vs2 = {1, 2, 4, 4, 2, 1, 2, 4};
  EXPECT_EQ(vs, vs2);
}

struct S {int a; float b;};
bool operator == (const S& left, const S& right) {
  return left.a == right.a && left.b == right.b;
}

TEST(renderer_test, VectorOverride) {
  std::vector<S> copy_from = {{1, 1.1}, {2, 2.2}, {3, 3.3}, {4, 4.4}};
  std::vector<S> copy_to = {{5, 5.5}, {6, 6.6}, {7, 7.7}};
  util::VectorOverride(&copy_to, 1, copy_from, 1, 3);
  std::vector<S> copy_result = {{5, 5.5}, {2, 2.2}, {3, 3.3}};
  EXPECT_EQ(copy_to, copy_result);
}

TEST(renderer_test, math) {
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    math::QuickSelect(&vs, 0, vs.size(), 3, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    math::QuickSelect(&vs, 0, vs.size(), 0, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    math::QuickSelect(&vs, 0, vs.size(), 1, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 2, 3, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    math::QuickSelect(&vs, 0, vs.size(), 2, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 2, 3, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    math::QuickSelect(&vs, 0, vs.size(), 4, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 7, 9, 6, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    math::QuickSelect(&vs, 0, vs.size(), 5, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 6, 7, 9, 8};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    math::QuickSelect(&vs, 0, vs.size(), 8, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    math::QuickSelect(&vs, 0, vs.size(), 7, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(vs, target);
  }
  {
    std::vector<int> vs = {4, 8, 2, 3, 5, 7, 9, 6, 1};
    math::QuickSelect(&vs, 0, vs.size(), 6, [](int l, int r){ return l <= r; });
    std::vector<int> target = {1, 3, 2, 4, 5, 6, 7, 9, 8};
    EXPECT_EQ(vs, target);
  }
}

TEST(renderer_test, mesh_repo_bootstraps_without_active_rhi_device) {
  cg::MeshRepo mesh_repo;
  EXPECT_TRUE(mesh_repo.Has("cube"));
  EXPECT_TRUE(mesh_repo.Has("plane"));
  EXPECT_TRUE(mesh_repo.Has("sphere"));
  EXPECT_TRUE(mesh_repo.Has("coordinator"));

  const cg::Mesh* cube_mesh = mesh_repo.GetMesh(mesh_repo.GetIndex("cube"));
  ASSERT_NE(cube_mesh, nullptr);
  EXPECT_FALSE(cube_mesh->positions().empty());
  EXPECT_FALSE(cube_mesh->normals().empty());
  EXPECT_FALSE(cube_mesh->texcoords().empty());
}

TEST(renderer_test, texture2d_keeps_cpu_payload_without_active_rhi_device) {
  std::vector<glm::vec4> pixels = {
      {1.0f, 0.5f, 0.25f, 1.0f},
      {0.0f, 0.25f, 0.75f, 1.0f},
  };
  cg::Texture texture = cg::CreateTexture2D(2, 1, pixels);

  EXPECT_FALSE(texture.empty());
  EXPECT_EQ(texture.meta().width, 2);
  EXPECT_EQ(texture.meta().height, 1);

  const std::vector<float> readback = texture.GetData<float>();
  ASSERT_EQ(readback.size(), pixels.size() * 4);
  EXPECT_FLOAT_EQ(readback[0], 1.0f);
  EXPECT_FLOAT_EQ(readback[1], 0.5f);
  EXPECT_FLOAT_EQ(readback[2], 0.25f);
  EXPECT_FLOAT_EQ(readback[3], 1.0f);
  EXPECT_FLOAT_EQ(readback[4], 0.0f);
  EXPECT_FLOAT_EQ(readback[5], 0.25f);
  EXPECT_FLOAT_EQ(readback[6], 0.75f);
  EXPECT_FLOAT_EQ(readback[7], 1.0f);
}

TEST(renderer_test, texture2d_array_builds_in_stable_index_order_without_active_rhi_device) {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = 1;
  meta.height = 1;
  meta.channel_num = 4;
  meta.hdr = false;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = cg::rhi::TextureFormat::kRGBA8;
  meta.pixel_format = cg::rhi::PixelFormat::kRGBA;
  meta.pixel_type = cg::rhi::PixelType::kUInt8;
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kRepeat;
  meta.wrap_t = cg::rhi::WrapMode::kRepeat;

  std::vector<unsigned char> pixel0 = {1, 2, 3, 4};
  std::vector<unsigned char> pixel1 = {5, 6, 7, 8};

  cg::TextureRepo texture_repo;
  texture_repo.AddUnique("a", cg::CreateTexture2D(meta, {pixel0.data()}));
  texture_repo.AddUnique("b", cg::CreateTexture2D(meta, {pixel1.data()}));

  const cg::Texture array_texture = texture_repo.AsTexture2DArray(1, 1);
  const std::vector<unsigned char> readback = array_texture.GetData<unsigned char>();
  EXPECT_EQ(readback, (std::vector<unsigned char>{1, 2, 3, 4, 5, 6, 7, 8}));
}

TEST(renderer_test, texture_repo_loads_named_texture2d_and_cubemap_configs_lazily) {
  cg::Config config;
  config.Init(util::FileJoin(CG_PROJECT_SOURCE_DIR, "playground/config.pb.txt"));

  cg::TextureRepo texture_repo;
  texture_repo.Init(config);

  EXPECT_TRUE(texture_repo.Has("brickwall"));
  const cg::Texture& brickwall = texture_repo.GetTexture("brickwall");
  EXPECT_EQ(brickwall.meta().type, cg::Texture::kTexture2D);
  EXPECT_EQ(brickwall.meta().level_num, 1);

  EXPECT_TRUE(texture_repo.Has("skybox"));
  const cg::Texture& skybox = texture_repo.GetTexture("skybox");
  EXPECT_EQ(skybox.meta().type, cg::Texture::kCubemap);
  EXPECT_EQ(skybox.meta().level_num, 1);
  EXPECT_EQ(skybox.meta().depth, 6);

  const cg::Texture& cubemap_lod = texture_repo.GetTexture("cubemap_lod_rgb");
  EXPECT_EQ(cubemap_lod.meta().type, cg::Texture::kCubemap);
  EXPECT_EQ(cubemap_lod.meta().level_num, 3);
  EXPECT_EQ(cubemap_lod.meta().depth, 6);
}
