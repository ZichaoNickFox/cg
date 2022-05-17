package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library (
  name = "gl",
  srcs = [
    "gl.h",
    "gl.cc"
  ],
  deps = [
    "//engine:debug",
    "@com_github_google_glog//:glog",
  ],
)

cc_library (
  name = "debug",
  srcs = [
    "debug.h"
  ],
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
  ],
)

cc_library(
  name = "engine",
  srcs = [
    "camera.h",
    "camera.cc",
    "io.h",
    "io.cc",
    "material.h",
    "material.cc",
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
    "@imgui//:imgui",
    "@fmt//:fmt",
  ],
  linkopts = [
    "-lGLEW",
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
    "@com_github_google_glog//:glog"
  ],
)