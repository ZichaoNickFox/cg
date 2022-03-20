package(default_visibility=["//visibility:public"])
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
  name = "object",
  srcs = [
    "cube.h",
    "cube.cc",
    "plane.h",
    "plane.cc",
    "lines.h",
    "lines.cc"
  ],
  deps = [
    "//engine:engine",
    "//playground:context",
  ],
  linkopts = [
    "-lfmt",
    "-lglog"
  ],
)