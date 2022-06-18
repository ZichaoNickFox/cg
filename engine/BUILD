package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library (
  name = "gl",
  srcs = [
    "gl.h",
    "gl.cc",
  ],
  deps = [
    "//engine:debug",
    "@glew",
  ],
)

cc_library (
  name = "debug",
  srcs = [
    "debug.h"
  ],
  deps = [
    "@boost//:stacktrace",
    "@com_github_google_glog//:glog"
  ],
  defines = select({
    "@platforms//os:windows": [
      "CG_PLATFORM_WINDOWS",
    ],
    "@platforms//os:macos": [
      "CG_PLATFORM_MACOS"
    ],
    "//conditions:default": [
    ],
  }),
)

cc_library(
  name = "engine",
  srcs = [
    "any.h",
    "automic_counter.h",
    "automic_counter.cc",
    "bvh.h",
    "bvh.cc",
    "camera.h",
    "camera.cc",
    "color.h",
    "compute_shader.h",
    "compute_shader.cc",
    "constants.h",
    "frame_stat.h",
    "frame_stat.cc",
    "framebuffer.h",
    "framebuffer.cc",
    "framebuffer_attachment.h",
    "framebuffer_attachment.cc",
    "geometry.h",
    "geometry.cc",
    "io.h",
    "io.cc",
    "material.h",
    "material.cc",
    "math.h",
    "math.cc",
    "mesh.h",
    "mesh.cc",
    "oneof.h",
    "pass.h",
    "pass.cc",
    "primitive.h",
    "shader.h",
    "shader.cc",
    "shader_loader.h",
    "shader_loader.cc",
    "SSBO.h",
    "SSBO.cc",
    "texture.h",
    "texture.cc",
    "texture_buffer.h",
    "texture_buffer.cc",
    "transform.h",
    "transform.cc",
  ],
  deps = [
    "//engine:debug",
    "//engine:gl",
    "//engine:util",
    "@assimp",
    "@glm",
    "@imgui",
    "@ordered_map",
  ],
)

cc_library(
  name = "util",
  srcs = [
    "util.h",
    "util.cc",
  ],
  deps = [
    "//engine:debug",
    # TODO : for compile success
    "//engine/proto:cc_config_proto",
    "@fmt",
    "@glm",
  ],
)

cc_test (
  name = "any_test",
  srcs = [
    "any_test.cc",
  ],
  deps = [
    "//engine:engine",
    "//engine:util",
    "@gtest//:gtest",
    "@gtest//:gtest_main"
  ],
)

cc_test (
  name = "oneof_test",
  srcs = [
    "oneof_test.cc",
  ],
  deps = [
    "//engine:engine",
    "//engine:util",
    "@gtest//:gtest",
    "@gtest//:gtest_main"
  ],
)