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
  name = "mesh",
  srcs = [
    "mesh.h",
    "mesh.cc",
  ],
  deps = [
    "//engine:debug",
    "//engine:gl",
    "@glm",
  ],
)

cc_library(
  name = "engine",
  srcs = [
    "camera.h",
    "camera.cc",
    "framebuffer.h",
    "framebuffer.cc",
    "framebuffer_attachment.h",
    "framebuffer_attachment.cc",
    "io.h",
    "io.cc",
    "material.h",
    "material.cc",
    "math.h",
    "shader.h",
    "shader.cc",
    "texture.h",
    "texture.cc",
    "transform.h",
    "transform.cc",
  ],
  deps = [
    "//engine:debug",
    "//engine:gl",
    "//engine/primitive:primitive",
    "//engine:util",
    "@fmt",
    "@glm",
    "@imgui",
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