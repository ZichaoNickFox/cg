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
    "//engine:gl",
    "//playground:playground",
    "@fmt",
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
    "//engine:engine",
    "//engine/proto:cc_config_proto",
    "//playground:scene",
    "//playground/scene:scenes",
    "@fmt",
  ],
)

cc_library(
  name = "scene",
  srcs = [
    "scene.h"
  ],
  deps = [
    "//playground:context",
  ],
)

cc_library(
  name = "context",
  srcs = [
    "context.h",
    "context.cc",
  ],
  deps = [
    "//engine:engine",
    "//engine/primitive:primitive",
    "//engine/proto:cc_config_proto",
    "//engine/repo:repo",
    "//engine:util",
    "@fmt",
  ],
)

cc_library(
  name = "shaders",
  srcs = [
    "shaders.h",
    "shaders.cc",
  ],
  deps = [
    "//engine:engine",
    "//engine/path_tracing:path_tracing",
    "//engine:util",
    "//playground:context",
    "//playground/object:object",
    "@glm//:glm",
  ]
)

cc_test (
  name = "test",
  srcs = [
      "test.cc",
  ],
  deps = [
    "//engine:util",
    "@gtest//:gtest",
    "@gtest//:gtest_main"
  ],
)