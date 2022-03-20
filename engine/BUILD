package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

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
    "shader_repo.h",
    "shader_repo.cc",
    "texture.h",
    "texture.cc",
    "texture_repo.h",
    "texture_repo.cc",
    "transform.h",
    "transform.cc",
    "util.h",
    "util.cc",
  ],
  deps = [
    "//engine:cc_config_proto",
    "@SOIL2//:SOIL2",
  ],
  includes = [
  ],
  linkopts = [
    "-lfmt",
    "-lGLEW",
  ],
)

proto_library(
  name = "config_proto",
  srcs = [ "config.proto" ]
)

cc_proto_library(
  name = "cc_config_proto",
  deps = [ ":config_proto" ]
)

cc_test (
  name = "engine_test",
  srcs = [
      "engine_test.cc",
  ],
  deps = [
    "//engine:engine",
    "@gtest//:gtest",
    "@gtest//:gtest_main"
  ],
)