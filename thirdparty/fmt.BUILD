package(default_visibility=["//visibility:public"])
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "fmt",
  srcs = [
    "src/format.cc",
  ],
  hdrs = [
    "include/fmt/core.h",
    "include/fmt/format.h",
    "include/fmt/format-inl.h",
  ],
  includes = [
    "include",
  ]
)
