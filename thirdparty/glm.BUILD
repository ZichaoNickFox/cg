package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "glm",
  hdrs = glob([
    "glm/*.hpp",
    "glm/**/*.hpp",
  ]),
  includes = ["glm"],
  textual_hdrs = glob(["glm/**/*.inl"]),
  visibility = ["//visibility:public"],
)