package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//:cg_proto_library.bzl", "cg_proto_library")

cc_binary(
  name = "window",
  srcs = [
    "window.cc",
  ],
  deps = [
    "//base:debug",
    "//renderer:gl",
    "//playground:playground",
    "@glew",
    "@glfw",
    "@glm",
    "@imgui",
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
  name = "playground",
  srcs = [
    "playground.h",
    "playground.cc",
  ],
  deps = [
    "//renderer:renderer",
    "//playground/geometry_scene:geometry_scene",
    "//playground/physics_scene:physics_scene",
    "//playground/renderer_scene:renderer_scene",
  ],
)