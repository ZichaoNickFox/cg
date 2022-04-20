package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//:cg_proto_library.bzl", "cg_proto_library")

cc_binary(
  name = "window",
  srcs = [
    "window.cc",
  ],
  deps = [
    "//engine:debug",
    "//playground:playground",
    "@imgui//:imgui",
    "@imgui//backends:backends",
  ],
  linkopts = [
    "-lfmt",
    "-lGLEW",
    "-lglfw3",
    "-framework OpenGL",
    "-framework Cocoa",
    "-framework IOKit",
    "-framework CoreFoundation",
  ],
)

cc_library(
  name = "playground",
  srcs = [
    "playground.h",
    "playground.cc",
  ],
  deps = [
    "//engine:engine",
    "//playground:scene",
    "//playground/proto:cc_config_proto",
    "//playground/scene:scenes",
  ],
  linkopts = [
    "-lfmt",
    "-lassimp",
  ],
)

cc_library(
  name = "scene",
  srcs = [
    "scene.h"
  ],
  deps = [
    "//playground:context",
  ]
)
cc_library(
  name = "context",
  srcs = [
    "context.h",
    "context.cc",
    "io.h",
    "io.cc",
    "mesh_repo.h",
    "mesh_repo.cc",
    "model_repo.h",
    "model_repo.cc",
    "shader_repo.h",
    "shader_repo.cc",
    "texture_repo.h",
    "texture_repo.cc",
  ],
  deps = [
    "//engine:engine",
    "//playground:util",
    "//playground/primitive:primitives",
    "//playground/proto:cc_config_proto",
    "@SOIL2//:SOIL2",
  ],
  linkopts = [
    "-lfmt",
  ],
)

cc_library(
  name = "pass",
  srcs = [
    "pass.h",
    "pass.cc",
    "shaders.h",
    "shaders.cc",
  ],
  deps = [
    "//engine:engine",
    "//playground:context",
    "//playground/object:object",
    "//playground:util",
  ]
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
    "//playground/proto:cc_config_proto",
  ],
  linkopts = [
    "-lglog"
  ],
)

cc_test (
  name = "test",
  srcs = [
      "test.cc",
  ],
  deps = [
    "//playground:util",
    "@gtest//:gtest",
    "@gtest//:gtest_main"
  ],
)