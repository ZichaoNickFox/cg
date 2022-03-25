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
    "util.h",
    "util.cc",
  ],
  deps = [
    "//engine:engine",
    "//playground:cc_config_proto",
    "@SOIL2//:SOIL2",
  ],
  linkopts = [
    "-lfmt",
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