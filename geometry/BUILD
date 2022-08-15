package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library (
  name = "geometry",
  srcs = [
    "plot.h",
    "plot.cc",
  ],
  deps = [
    "//base:base",
    "@glew",
    "@implot",
  ],
)
