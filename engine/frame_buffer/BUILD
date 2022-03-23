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