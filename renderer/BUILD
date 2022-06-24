package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library (
  name = "gl",
  srcs = [
    "gl.h",
    "gl.cc",
  ],
  deps = [
    "//renderer:debug",
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
  name = "renderer",
  srcs = [
    "automic_counter.h",
    "automic_counter.cc",
    "bvh.h",
    "bvh.cc",
    "camera.h",
    "camera.cc",
    "color.h",
    "compute_shader.h",
    "compute_shader.cc",
    "config.h",
    "config.cc",
    "definition.h",
    "filter.h",
    "filter.cc",
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
    "light.h",
    "light.cc",
    "material.h",
    "material.cc",
    "math.h",
    "math.cc",
    "mesh.h",
    "mesh.cc",
    "meshes/cube_mesh.h",
    "meshes/cube_mesh.cc",
    "meshes/empty_mesh.h",
    "meshes/empty_mesh.cc",
    "meshes/lines_mesh.h",
    "meshes/lines_mesh.cc",
    "meshes/mesh_util.h",
    "meshes/mesh_util.cc",
    "meshes/plane_mesh.h",
    "meshes/plane_mesh.cc",
    "meshes/sphere_mesh.h",
    "meshes/sphere_mesh.cc",
    "model_loader.h",
    "model_loader.cc",
    "object.h",
    "object.cc",
    "pass.h",
    "pass.cc",
    "primitive.h",
    "primitive.cc",
    "render_shader.h",
    "render_shader.cc",
    "scene.h",
    "scene.cc",
    "scene_common.h",
    "scene_common.cc",
    "shader.h",
    "shader.cc",
    "shaders.h",
    "shaders.cc",
    "shader_loader.h",
    "shader_loader.cc",
    "shadow.h",
    "shadow.cc",
    "ssbo.h",
    "ssbo.cc",
    "texture.h",
    "texture.cc",
    "texture_buffer.h",
    "texture_buffer.cc",
    "transform.h",
    "transform.cc",
  ],
  deps = [
    "//renderer:debug",
    "//renderer:gl",
    "//renderer:util",
    "@assimp",
    "@glm",
    "@imgui",
    "@ordered_map",
    "@stb"
  ],
)

cc_library(
  name = "util",
  srcs = [
    "util.h",
    "util.cc",
  ],
  deps = [
    "//renderer:debug",
    # TODO : for compile success
    "//renderer/proto:cc_config_proto",
    "@fmt",
    "@glm",
  ],
)
