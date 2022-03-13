package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")


cc_binary(
  name = "playground",
  srcs = [
    "playground.cc",
  ],
  deps = [
    "//playground:framework",
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
  name = "io_input",
  srcs = [
    "io_input.h",
  ],
)

cc_library(
  name = "framework",
  srcs = [
    "framework.h",
    "framework.cc",
  ],
  deps = [
    "//engine:engine",
    "//playground:cc_scene_proto",
    "//playground:io_input",
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
  ],
  deps = [
    "//engine:engine",
    "//playground:cc_scene_proto"
  ],
  linkopts = [
    "-lfmt",
  ],
)

proto_library(
  name = "scene_proto",
  srcs = [ "scene.proto" ]
)

cc_proto_library(
  name = "cc_scene_proto",
  deps = [ ":scene_proto" ]
)