package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "frame_buffer",
  srcs = [
    "color_frame_buffer.h",
    "color_frame_buffer.cc",
    "depth_frame_buffer.h",
    "depth_frame_buffer.cc",
    "frame_buffer.h",
    "frame_buffer.cc",
    "frame_buffer_render.h",
    "frame_buffer_render.cc",
  ],
  deps = [
    "//engine:engine",
    "@SOIL2//:SOIL2",
  ],
  includes = [
  ],
  linkopts = [
    "-lfmt",
    "-lGLEW",
  ],
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