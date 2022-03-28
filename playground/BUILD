package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
  name = "launcher",
  srcs = [
    "launcher.cc",
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
    "//playground:cc_config_proto",
    "//playground:scene",
    "//playground/scene:scenes",
  ],
  linkopts = [
    "-lfmt",
  ],
)

cc_library(
  name = "scene",
  srcs = [
    "scene.h"
  ],
  deps = [
    "//playground:context"
  ]
)
cc_library(
  name = "context",
  srcs = [
    "context.h",
    "context.cc",
    "io.h",
    "io.cc",
    "shader_repo.h",
    "shader_repo.cc",
    "texture_repo.h",
    "texture_repo.cc",
  ],
  deps = [
    "//engine:engine",
    "//playground:cc_config_proto",
    "//playground:util",
    "@SOIL2//:SOIL2",
  ],
  linkopts = [
    "-lfmt",
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
    "//playground:cc_config_proto",
  ],
  linkopts = [
    "-lglog"
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