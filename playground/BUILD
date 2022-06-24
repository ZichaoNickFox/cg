package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//:cg_proto_library.bzl", "cg_proto_library")

cc_binary(
  name = "window",
  srcs = [
    "window.cc",
  ],
  deps = [
    "//renderer:debug",
    "//renderer:gl",
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
    "//renderer:renderer",
    "//renderer/proto:cc_config_proto",
    "//playground/scene:scenes",
    "@fmt",
  ],
)

cc_test (
  name = "test",
  srcs = [
      "test.cc",
  ],
  deps = [
    "//renderer:renderer",
    "//renderer:util",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
    "@glew",
    "@glm",
  ],
)