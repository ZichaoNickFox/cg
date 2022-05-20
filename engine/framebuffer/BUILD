package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "framebuffer",
  srcs = [
    "color_framebuffer.h",
    "color_framebuffer.cc",
    "depth_framebuffer.h",
    "depth_framebuffer.cc",
    "gbuffer.h",
    "gbuffer.cc",
    "ms_framebuffer.h",
    "ms_framebuffer.cc",
  ],
  deps = [
    "//engine:debug",
    "//engine:engine",
    "//engine:util",
    "@fmt//:fmt",
  ],
)