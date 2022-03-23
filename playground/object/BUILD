package(default_visibility=["//visibility:public"])
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
  name = "object",
  srcs = [
    "billboard.h",
    "billboard.cc",
    "cube.h",
    "cube.cc",
    "directional_light.h",
    "directional_light.cc",
    "object.h",
    "plane.h",
    "plane.cc",
    "lines.h",
    "lines.cc"
  ],
  deps = [
    "//engine:engine",
    "//engine/frame_buffer:frame_buffer",
    "//playground:context",
  ],
  linkopts = [
    "-lfmt",
    "-lglog"
  ],
)