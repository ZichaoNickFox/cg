package(default_visibility=["//visibility:public"])
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
  name = "scenes",
  srcs = [
    "common.h",
    "common.cc",
    "cube_world_scene.h",
    "cube_world_scene.cc",
    "imgui_demo_scene.h",
    "imgui_demo_scene.cc",
    "gallery_scene.h",
    "gallery_scene.cc",
    "test_scene.h",
    "test_scene.cc",
    "triangle_scene.h",
    "triangle_scene.cc",
  ],
  deps = [
    "//playground:context",
    "//playground:scene",
    "@imgui//:imgui",
  ],
  linkopts = [
    "-lfmt",
    "-lglog"
  ],
)