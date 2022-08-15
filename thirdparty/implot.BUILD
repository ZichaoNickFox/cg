package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "implot",
  srcs = [
    "implot.h",
    "implot.cpp",
    "implot_demo.cpp",
    "implot_internal.h",
    "implot_items.cpp",
  ],
  deps = [
    "@imgui"
  ]
)