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
    "frame_buffer_attachment.h",
    "g_buffer.h",
    "g_buffer.cc",
    "ms_frame_buffer.h",
    "ms_frame_buffer.cc",
  ],
  deps = [
    "//engine:debug",
    "//engine:engine",
    "//engine:util",
  ],
  linkopts = [
    "-lfmt",
    "-lGLEW",
  ],
)